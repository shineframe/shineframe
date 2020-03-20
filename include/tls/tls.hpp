#pragma once
#include <iostream>
#include <functional>
#include <openssl/ssl.h>
#include <openssl/err.h>

namespace shine {
	namespace tls {
		static inline bool init() {
			if (!SSL_library_init()) {
				return false;
			}

			SSL_load_error_strings();
			return true;
		}

		static inline SSL_CTX * create_context(const SSL_METHOD *meth, const char *cert, const char *key, bool verify_peer = false, const char *ca = NULL) {
			SSL_CTX *ctx = SSL_CTX_new(meth);
			if (ctx == NULL) {
				return nullptr;
			}

			if (cert == NULL || key == NULL) {
				return ctx;
			}

			if (SSL_CTX_use_certificate_file(ctx, cert, SSL_FILETYPE_PEM) <= 0) {
				goto FAILED;
			}

			if (SSL_CTX_use_PrivateKey_file(ctx, key, SSL_FILETYPE_PEM) <= 0) {
				goto FAILED;
			}

			if (SSL_CTX_check_private_key(ctx) != 1) {
				goto FAILED;
			}

			if (verify_peer) {
				if (ca == NULL) {
					goto FAILED;
				}

				if (!SSL_CTX_load_verify_locations(ctx, ca, NULL)) {
					goto FAILED;
				}

				SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
				SSL_CTX_set_verify_depth(ctx, 1);
			}

			return ctx;

		FAILED:
			if (ctx != NULL) {
				SSL_CTX_free(ctx);
			}

			return NULL;
		}

		typedef std::function<void(const char * data, size_t len)> send_func_t;
		typedef std::function<void(int error_no, const char *error_msg)> on_handshake_callback_func_t;

		class base {
		public:
			base(SSL_CTX *ctx_, bool verify_peer_, send_func_t send_func_, on_handshake_callback_func_t on_handshake_callback_func_) {
				ctx = ctx_;
				verify_peer = verify_peer_;
				send_func = send_func_;
				on_handshake_callback_func = on_handshake_callback_func_;

				ssl = SSL_new(ctx);
				read_bio = BIO_new(BIO_s_mem());
				write_bio = BIO_new(BIO_s_mem());
				SSL_set_bio(ssl, read_bio, write_bio);

				buf.resize(1024);
			}

            ~base(){
                if (ssl != NULL)
                {
                    SSL_free(ssl);
                }
            }

			bool is_handshake_finished() {
				return SSL_is_init_finished(ssl);
			}

			bool send(const char *data, size_t len) {
				if (len <= 0) {
					return false;
				}

				if (!is_handshake_finished()) {
					return false;
				}

				int w_len = SSL_write(ssl, data, len);
				if (w_len < len) {
					return false;
				}

				std::string out;

				for (;;) {
					int r_len = BIO_read(write_bio, (void*)buf.data(), buf.size());
					if (r_len > 0) {
						out.append(buf.data(), r_len);
						if (r_len < buf.size())
						{
							break;
						}
					}
					else {
						int err = SSL_get_error(ssl, r_len);
						return err == SSL_ERROR_NONE;
					}
				}

				send_func(out.data(), out.size());
				return true;
			}

			bool recv(std::string &pure_data, const char *data, size_t len) {
				if (len > 0){
					int w_len = BIO_write(read_bio, data, len);
				}

				if (!is_handshake_finished()) {
					int rc = SSL_do_handshake(ssl);
                    for (;;)
                    {
                        int r_len = BIO_read(write_bio, (void*)buf.data(), buf.size());
                        if (r_len > 0) {
                            send_func(buf.data(), r_len);
                            if (r_len < buf.size())
                            {
                                break;
                            }
                        }
                        else {
                            if (rc != 1)
                            {
                                int err = SSL_get_error(ssl, r_len);
                                if (err == SSL_ERROR_NONE || err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE) {
                                    return true;
                                }
                                else {
                                    //error
									on_handshake_callback_func(err, "Handshake error")
                                    return false;
                                }
                            }
                            else{
                                break;
                            }
                        }
                    }

					if (rc != 1) {
						int err = SSL_get_error(ssl, rc);
                        if (err == SSL_ERROR_NONE || err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE) {
							return true;
						}
						else {
							//error
                            on_handshake_callback_func(err, "Handshake error")
							return false;
						}
					}
					else {
						if (verify_peer)
						{
							X509 *ssl_client_cert = NULL;

							ssl_client_cert = SSL_get_peer_certificate(ssl);

							if (ssl_client_cert)
							{
								long verifyresult;

								verifyresult = SSL_get_verify_result(ssl);
								if (verifyresult == X509_V_OK) {
                                    on_handshake_callback_func(verifyresult, "Handshake success");
								}
								else {
                                    on_handshake_callback_func(verifyresult, "Certificate verify Failed");
                                    return false;
								}
								X509_free(ssl_client_cert);
							}
							else {
                                on_handshake_callback_func(-1, "There is no certificate");
                                return false;
							}
						}
					}
				}
				else {
					for (;;)
					{
						int r_len = SSL_read(ssl, (void*)buf.data(), buf.size());
						if (r_len > 0){
							pure_data.append(buf.data(), r_len);
							if (r_len < buf.size())
							{
								return true;
							}
						}
						else {
							int err = SSL_get_error(ssl, r_len);

							if (err == SSL_ERROR_NONE || err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE) {
								return true;
							}
							else {
								//error
								return false;
							}
						}

					}
				}

				return true;
			}

		public:
			SSL_CTX *ctx;
			SSL *ssl;
			BIO *read_bio;
			BIO *write_bio;
			bool verify_peer;
			std::string buf;
			send_func_t send_func;
			on_handshake_callback_func_t on_handshake_callback_func;
		};

		class server : public base {
		public:
			server(SSL_CTX *ctx_, bool verify_, send_func_t send_func_) : base(ctx_, verify_, send_func_) {
				SSL_set_accept_state(ssl);
			}

			void on_accept() {

			}
		};

		class client : public base {
		public:
			client(SSL_CTX *ctx_, bool verify_, send_func_t send_func_) : base(ctx_, verify_, send_func_) {
				SSL_set_connect_state(ssl);
			}

			void on_connect() {
				std::string tmp;
				recv(tmp, nullptr, 0);
			}
		};

#if (0)
		inline void test() {
			base::init();
			SSL_CTX *server_ctx = base::create_context(SSLv23_method(), "cert.crt", "server.key");
			SSL_CTX *client_ctx = base::create_context(SSLv23_method(), NULL, NULL);


			server server(server_ctx, false, [](const char* data, size_t len) {});
			client client(client_ctx, false, [](const char* data, size_t len) {});;
			char buf[2048];
			int ret;
			int len;

			bool server_good = false;
			bool client_good = false;
			for (; ;)
			{
				if (!SSL_is_init_finished(client.ssl)) {
					ret = SSL_do_handshake(client.ssl);
					if (ret != 1)
					{
						len = BIO_read(client.write_bio, buf, sizeof(buf));
						len = BIO_write(server.read_bio, buf, len);
					}
				}
				else {
					client_good = true;
				}

				if (!SSL_is_init_finished(server.ssl)) {
					ret = SSL_do_handshake(server.ssl);
					//if (ret != 1)
					{
						len = BIO_read(server.write_bio, buf, sizeof(buf));
						len = BIO_write(client.read_bio, buf, len);
					}
				}
				else {
					server_good = true;
				}

				if (client_good && server_good)
				{
					break;
				}
			}


			std::string data;
			std::string tmp;
			tmp.resize(10240000);
			bool rc = server.send(data, tmp.data(), tmp.size());
			std::cout << rc << std::endl;
		}
#endif
	}
}