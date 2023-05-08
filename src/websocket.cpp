#include "websocket.h"

#include "log.h"
#include "ssml-creator.h"

#include <iostream>
#include <fstream>
#include <openssl/ssl.h>
#include <assert.h>

WebSocketConnection* WebSocketConnection::WSConn = nullptr;

static inline const char* LwsCallbackReasonToString(lws_callback_reasons Reason)
{
    switch (Reason)
    {
#define CASE_ENUM_RETURN_STRING(CaseEnum)\
    case CaseEnum: return #CaseEnum;

        CASE_ENUM_RETURN_STRING(LWS_CALLBACK_CLIENT_CONNECTION_ERROR);
        CASE_ENUM_RETURN_STRING(LWS_CALLBACK_CLIENT_FILTER_PRE_ESTABLISH);
        CASE_ENUM_RETURN_STRING(LWS_CALLBACK_CLIENT_ESTABLISHED);
        CASE_ENUM_RETURN_STRING(LWS_CALLBACK_CLOSED);
        CASE_ENUM_RETURN_STRING(LWS_CALLBACK_CLIENT_RECEIVE);
        CASE_ENUM_RETURN_STRING(LWS_CALLBACK_CLIENT_WRITEABLE);
        CASE_ENUM_RETURN_STRING(LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS);
        CASE_ENUM_RETURN_STRING(LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER);
        CASE_ENUM_RETURN_STRING(LWS_CALLBACK_PROTOCOL_INIT);
        CASE_ENUM_RETURN_STRING(LWS_CALLBACK_WSI_CREATE);
        CASE_ENUM_RETURN_STRING(LWS_CALLBACK_WSI_DESTROY);
        CASE_ENUM_RETURN_STRING(LWS_CALLBACK_GET_THREAD_ID);
        CASE_ENUM_RETURN_STRING(LWS_CALLBACK_ADD_POLL_FD);
        CASE_ENUM_RETURN_STRING(LWS_CALLBACK_DEL_POLL_FD);
        CASE_ENUM_RETURN_STRING(LWS_CALLBACK_CHANGE_MODE_POLL_FD);
        CASE_ENUM_RETURN_STRING(LWS_CALLBACK_LOCK_POLL);
        CASE_ENUM_RETURN_STRING(LWS_CALLBACK_UNLOCK_POLL);
        CASE_ENUM_RETURN_STRING(LWS_CALLBACK_OPENSSL_PERFORM_SERVER_CERT_VERIFICATION);
        CASE_ENUM_RETURN_STRING(LWS_CALLBACK_CLIENT_CLOSED);
#undef CASE_ENUM_RETURN_STRING

#define CASE_ENUM_RETURN_STRING_S(CaseEnum)\
            case CaseEnum: return "---"#CaseEnum;

		//CASE_ENUM_RETURN_STRING_S(LWS_CALLBACK_CHANGE_MODE_POLL_FD);
		//CASE_ENUM_RETURN_STRING_S(LWS_CALLBACK_LOCK_POLL);
		//CASE_ENUM_RETURN_STRING_S(LWS_CALLBACK_UNLOCK_POLL);
#undef CASE_ENUM_RETURN_STRING_S
    }
    return std::to_string(Reason).c_str();
}

int WebSocketConnection::StaticLwsCallback(lws* Connection, lws_callback_reasons Reason, void* UserData, void* Data, size_t Length)
{
    vvr_log(LwsCallbackReasonToString(Reason));

    return WSConn->LwsCallback(Connection, Reason, UserData, Data, Length);
}

void WebSocketConnection::LwsLog(int Level, const char* LogLine)
{
    vvr_log("Callfrom libwebsocket, LogLevel: %d, Content: %s", Level, LogLine);
}

WebSocketConnection::WebSocketConnection()
{
    WSConn = this;
}

WebSocketConnection::~WebSocketConnection()
{
    if (WSConn == this) WSConn = nullptr;
}

void WebSocketConnection::Connect(char* ConnectionUrl)
{
    // Ini Lws.
    lws_protocols LwsProtocols[2];
    lws_protocols* LwsProtocol = &LwsProtocols[0];
    memset(LwsProtocol, 0, sizeof(lws_protocols));

    LwsProtocol->name = "ws";
    LwsProtocol->callback = &WebSocketConnection::StaticLwsCallback;
    LwsProtocol->per_session_data_size = 0;
    LwsProtocol->rx_buffer_size = 16384;

 //   LwsProtocol = &LwsProtocols[1];
 //   memset(LwsProtocol, 0, sizeof(lws_protocols));
	//LwsProtocol->name = "wss";
	//LwsProtocol->callback = &WebSocketConnection::StaticLwsCallback;
	//LwsProtocol->per_session_data_size = 0;
	//LwsProtocol->rx_buffer_size = 16384;

    lws_set_log_level(LLL_ERR | LLL_WARN | LLL_NOTICE | LLL_INFO | LLL_DEBUG | LLL_HEADER | LLL_EXT | LLL_CLIENT | LLL_LATENCY | LLL_USER, &WebSocketConnection::LwsLog);

    LwsProtocols[1] = { nullptr, nullptr, 0, 0 };
    // Ini ContextInfo.
    struct lws_context_creation_info ContextInfo = {};
    ContextInfo.port = CONTEXT_PORT_NO_LISTEN;
    ContextInfo.protocols = &LwsProtocols[0];
    ContextInfo.uid = -1;
    ContextInfo.gid = -1;
    ContextInfo.options |= LWS_SERVER_OPTION_PEER_CERT_NOT_REQUIRED | LWS_SERVER_OPTION_DISABLE_OS_CA_CERTS;
    ContextInfo.provided_client_ssl_ctx = nullptr;
    ContextInfo.max_http_header_data = 0;

    int MaxHttpHeaderData = 1024 * 32;
    ContextInfo.max_http_header_data2 = MaxHttpHeaderData;
    ContextInfo.pt_serv_buf_size = MaxHttpHeaderData;

    // Mark
    m_strCaFilePath = PROJECT_PATH + "resources/cacert.pem";
    m_strCertFilePath = PROJECT_PATH + "resources/clientCert.pem";
    m_strPrivateKeyFilePath = PROJECT_PATH + "resources/client-key.pem";
    ContextInfo.client_ssl_ca_filepath = m_strCaFilePath.c_str();
    ContextInfo.client_ssl_cert_filepath = m_strCertFilePath.c_str();
    ContextInfo.client_ssl_private_key_filepath = m_strPrivateKeyFilePath.c_str();
    ContextInfo.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
    //ContextInfo.provided_client_ssl_ctx = CreateSSLContext();

    LwsContext = lws_create_context(&ContextInfo);

    // Connect.

    const char* UrlProtocol;
    const char* TempUrlPath;
    const char* ParsedAddress;
    int ParsedPort;
    int Result = lws_parse_uri(ConnectionUrl, &UrlProtocol, &ParsedAddress, &ParsedPort, &TempUrlPath);

    size_t UrlPathLength = strlen(TempUrlPath) + 2;
    char* UrlPath = new char[UrlPathLength];
    UrlPath[0] = '/';
    memcpy(&UrlPath[1], TempUrlPath, strlen(TempUrlPath));
    UrlPath[UrlPathLength - 1] = '\0';

    int SslConnection = 0;
    if (strcmp(UrlProtocol, "ws") == 0)
    {
        SslConnection = 0;
    }
    else if (strcmp(UrlProtocol, "wss") == 0)
    {
        SslConnection = 1;
    }
    else if (strcmp(UrlProtocol, "wss+insecure") == 0)
    {
        SslConnection = 2;
    }
    else
    {
        vvr_log("Bad protocol.");
    }
    
    struct lws_client_connect_info ConnectInfo = {};
    ConnectInfo.context = LwsContext;
    ConnectInfo.address = ParsedAddress;
    ConnectInfo.port = ParsedPort;
    //ConnectInfo.ssl_connection = SslConnection;
    ConnectInfo.ssl_connection = LCCSCF_USE_SSL; // Mark
    ConnectInfo.path = UrlPath;
    ConnectInfo.host = ConnectInfo.address;
    //ConnectInfo.origin = "azure.microsoft.com";
    //ConnectInfo.origin = ConnectInfo.address;
    //ConnectInfo.protocol = "ws"; // Mark.
    ConnectInfo.ietf_version_or_minus_one = -1;
    ConnectInfo.userdata = nullptr; 

    if (lws_client_connect_via_info(&ConnectInfo))
    {
        vvr_log("Connection success!");
        bRunningFlag = true;
    }
    else
    {
        vvr_log("Connection failed!");
    }

    while (IsRunning())
    {
        Run();
    }

    delete UrlPath;
}

void WebSocketConnection::Run()
{
    if (LwsContext)
	{
        //if (lws_pollargs* Argsptr = SocketfdMrg.GetMaxArgs())
        //{
        //    lws_pollfd Pollfd;
        //    Pollfd.fd = Argsptr->fd;
        //    Pollfd.events = Argsptr->events;
        //    Pollfd.revents = Argsptr->prev_events;
        //    lws_service_fd(LwsContext, &Pollfd);
        //}

        lws_service(LwsContext, 0);
    }
}

int WebSocketConnection::LwsCallback(lws* Connection, lws_callback_reasons Reason, void* UserData, void* Data, size_t Length)
{
    switch (Reason)
	{
	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
	{
        if (Data != nullptr) vvr_log((char*)Data);
		break;
	}
    case LWS_CALLBACK_CLIENT_ESTABLISHED:
    {
        LwsConnection = Connection;
        lws_callback_on_writable(Connection);

        pSoundFile = new char[DEFAULT_OUTPUT_SOUND_FILE_LENGTH];
        nSoundFileLength = 0;

        break;
    }
    case LWS_CALLBACK_CLIENT_RECEIVE:
    {
        vvr_log("Receive Msg, Len: %d", Length);

        if (memcmp(Data, "X-RequestId", 11) == 0)
        {
            if (SendParamFlag > 1) lws_callback_on_writable(Connection);

            return 0;
        }

		//std::ofstream Outfile;
		//Outfile.open("../resources/test.mp3", std::ios::app/* | std::ios::binary*/);
		//if (!Outfile.is_open())
		//{
		//	return 0;
		//}

        for (int i = 0; i < Length; i++)
        {
            if (memcmp(((char*)Data) + i, "Path:audio", 10) == 0)
            {
                if (Length > (i + 12))
				{
					//Outfile.write(((const char*)Data) + i + 12, Length - i - 12);

                    memcpy(pSoundFile + nSoundFileLength, ((const char*)Data) + i + 12, Length - i - 12);
                    nSoundFileLength += Length - i - 12;

                    vvr_log("i: %d, Remain Length: %d", i, Length - i - 12);
                    break;
                }
            }
        }

        //Outfile.close();

        break;
    }
	case LWS_CALLBACK_CLIENT_WRITEABLE:
	{
		if (SendParamFlag == 0)
		{
            char Msg[1000];
			memset(&Msg[0], '\0', 1000);
			int MsgLen = sprintf_s(Msg, 1000, "Path: speech.config\r\nX-RequestId: %s\r\nX-Timestamp: 2023-03-07T09:29:27.929Z\r\nContent-Type: application/json\r\n\r\n{\"context\":{\"system\":{\"name\":\"SpeechSDK\",\"version\":\"1.19.0\",\"build\":\"JavaScript\",\"lang\":\"JavaScript\",\"os\":{\"platform\":\"Browser / Linux x86_64\",\"name\":\"Mozilla / 5.0 (X11; Linux x86_64; rv:78.0) Gecko / 20100101 Firefox / 78.0\",\"version\":\"5.0 (X11)\"}}}}", GetXConnectionID().c_str());
            WriteBuffer((uint8*)&Msg[0], MsgLen);

   //         char Msg[1000];
			//memset(&Msg[0], '\0', 1000);
			//int MsgLen = std::sprintf(&Msg[0], "Path: speech.config\r\nX-RequestId: %s\r\nX-Timestamp: 2023-03-01T08:00:08.863Z\r\nContent-Type: application/json\r\n\r\n{\"context\":{\"system\":{\"name\":\"SpeechSDK\",\"version\":\"1.19.0\",\"build\":\"JavaScript\",\"lang\":\"JavaScript\",\"os\":{\"platform\":\"Browser / Linux x86_64\",\"name\":\"Mozilla / 5.0 (X11; Linux x86_64; rv:78.0) Gecko / 20100101 Firefox / 78.0\",\"version\":\"5.0 (X11)\"}}}}", GetXConnectionID().c_str());
			//lws_write(Connection, (unsigned char*)&Msg[0], MsgLen - 1, lws_write_protocol::LWS_WRITE_BINARY);

            lws_callback_on_writable(Connection);
		}
		else if (SendParamFlag == 1)
		{
			char Msg[1000];
			memset(&Msg[0], '\0', 1000);
			int MsgLen = sprintf_s(Msg, 1000, "Path: synthesis.context\r\nX-RequestId: %s\r\nX-Timestamp: 2023-03-07T09:29:27.929Z\r\nContent-Type: application/json\r\n\r\n{\"synthesis\":{\"audio\":{\"metadataOptions\":{\"bookmarkEnabled\":false,\"sentenceBoundaryEnabled\":false,\"visemeEnabled\":false,\"wordBoundaryEnabled\":false},\"outputFormat\":\"audio-24khz-96kbitrate-mono-mp3\"},\"language\":{\"autoDetection\":false}}}", GetXConnectionID().c_str());
            WriteBuffer((uint8*)&Msg[0], MsgLen);

		}
		else if (SendParamFlag == 2)
		{
			char Msg[3000];
			memset(&Msg[0], '\0', 3000);
			int MsgLen = sprintf_s(Msg, 3000, "Path: ssml\r\nX-RequestId: %s\r\nX-Timestamp: 2023-03-07T09:29:27.929Z\r\nContent-Type: application/ssml+xml\r\n\r\n%s", GetXConnectionID().c_str(), SSMLCreatorModule::GetInstance()->GetSSMLFile());
			
            WriteBuffer((uint8*)&Msg[0], MsgLen);
		}
        else if (SendParamFlag == 4)
        {
            lws_close_reason(Connection, LWS_CLOSE_STATUS_NORMAL, nullptr, 0);
            return -1;
        }

        SendParamFlag++;
		break;
	}
    case LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER:
    {
        char** WriteableString = reinterpret_cast<char**>(Data);
        int CurrentSize = 0;

        char OriginHeader[] = "Origin: https://azure.microsoft.com\x0d\x0a";
        memcpy(*WriteableString, &OriginHeader[0], sizeof(OriginHeader));
        *WriteableString += sizeof(OriginHeader) - 1;

        char ExHeader[] = "Sec-WebSocket-Extensions: permessage-deflate; client_max_window_bits\x0d\x0a";
        memcpy((*WriteableString) + CurrentSize, &ExHeader[0], sizeof(ExHeader));
        *WriteableString += sizeof(ExHeader) - 1;

		char EncodingHeader[] = "Accept-Encoding: gzip, deflate, br\x0d\x0a";
		memcpy((*WriteableString) + CurrentSize, &EncodingHeader[0], sizeof(EncodingHeader));
        *WriteableString += sizeof(EncodingHeader) - 1;

		char LangHeader[] = "Accept-Language: zh-CN,zh;q=0.9\x0d\x0a";
		memcpy((*WriteableString) + CurrentSize, &LangHeader[0], sizeof(LangHeader));
        *WriteableString += sizeof(LangHeader) - 1;

		char UserAgentHeader[] = "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/110.0.0.0 Safari/537.36\x0d\x0a";
		memcpy((*WriteableString) + CurrentSize, &UserAgentHeader[0], sizeof(UserAgentHeader) -1);
        *WriteableString += sizeof(UserAgentHeader) - 1;
        break;
	}
	case LWS_CALLBACK_WSI_CREATE:
	{
		break;
	}
    case LWS_CALLBACK_ADD_POLL_FD:
    {
        lws_pollargs* PollArgs = (lws_pollargs*)Data;
        if (!SocketfdMrg.Exist(PollArgs->fd))
        {
            if (PollArgs->events & LWS_POLLIN)
            {
                SocketfdMrg.Add(PollArgs);
                vvr_log("poll in");
            }
        }
        else
        {
            if (PollArgs->events & LWS_POLLOUT)
            {
                vvr_log("poll out");
            }
            else
            {
                vvr_log("poll remove");
                SocketfdMrg.Remove(PollArgs->fd);
            }
        }

        break;
    }
    case LWS_CALLBACK_DEL_POLL_FD:
    {
        lws_pollargs* PollArgs = (lws_pollargs*)Data;
        if (SocketfdMrg.Exist(PollArgs->fd))
        {
			SocketfdMrg.Remove(PollArgs->fd);
            vvr_log("poll remove");
        }
        break;
    }
    case LWS_CALLBACK_CHANGE_MODE_POLL_FD:
    {
        lws_pollargs* PollArgs = (lws_pollargs*)Data;
        if (lws_pollargs* Args = SocketfdMrg.GetArgsByFd(PollArgs->fd))
        {
            Args->events = PollArgs->events;
            Args->prev_events = PollArgs->prev_events;
        }
        break;
    }
    case LWS_CALLBACK_OPENSSL_PERFORM_SERVER_CERT_VERIFICATION:
    {
        X509_STORE_CTX* Context = static_cast<X509_STORE_CTX*>(UserData);

        X509_STORE_CTX_set_error(Context, X509_V_OK);

        break;
  //      char* crl_path = "resources/ca.cer"; // path to root certifcate file

		///* Enable CRL checking of the server certificate */
		//X509_VERIFY_PARAM* param = X509_VERIFY_PARAM_new();
		//X509_VERIFY_PARAM_set_flags(param, X509_V_FLAG_TRUSTED_FIRST);
		//SSL_CTX_set1_param((SSL_CTX*)UserData, param);
		//X509_STORE* store = SSL_CTX_get_cert_store((SSL_CTX*)UserData);
		//X509_LOOKUP* lookup = X509_STORE_add_lookup(store, X509_LOOKUP_file());
		//int n = X509_load_cert_file(lookup, crl_path, X509_FILETYPE_PEM);
		//X509_VERIFY_PARAM_free(param);

		//if (n != 1) {
		//	char errbuf[256];
		//	n = ERR_get_error();
		//	lwsl_err("LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS: SSL error: %s (%d)\n", ERR_error_string(n, errbuf), n);
		//	return 1;
		//}
		//SSL_CTX_set_verify_depth((SSL_CTX*)UserData, 0); // when depth is set to 0 error is 21, when depth is 1 error is 27
		//SSL_CTX_set_verify((SSL_CTX*)UserData, SSL_VERIFY_NONE, mySSL_verify_callback);
    }
	case LWS_CALLBACK_CLIENT_CLOSED:
	{

		// Stop loop.
		bRunningFlag = false;

        if (nSoundFileLength == 0)
        {
            vvr_log("Sound file length 0, write failed.");
            break;
        }

        vvr_log("Begin write mp3 file, file length: %d", nSoundFileLength);

		std::ofstream Outfile;
		time_t currentTime = std::time(0);
		tm date;
		localtime_s(&date, &currentTime);
		char chFileName[LOG_FILE_NAME_MAX_LENGTH];
		sprintf_s(chFileName, "%d.%d.%d-%d.%d.%d", date.tm_year + 1900, date.tm_mon + 1, date.tm_mday, date.tm_hour, date.tm_min, date.tm_sec);
        String strFileName(chFileName);
        Outfile.open(OUTPUT_PATH + strFileName + ".mp3", std::ios::out | std::ios::binary);
        if (!Outfile.is_open())
        {
            vvr_log("Open file failed, write failed.");
	        return 0;
        }

        Outfile.write(pSoundFile, nSoundFileLength);

        Outfile.close();

        vvr_log("Write mp3 success, at %s", (OUTPUT_PATH + strFileName + ".mp3").c_str());

        delete pSoundFile;

		break;
	}
    }

    return 0;
}

void WebSocketConnection::WriteBuffer(uint8* Buffer, int Length)
{
    char* SendMessage = new char[Length + LWS_PRE];
    memcpy(SendMessage + LWS_PRE, Buffer, Length);
 
    lws_write(LwsConnection, ((unsigned char*)SendMessage) + LWS_PRE, Length, lws_write_protocol::LWS_WRITE_TEXT);

    delete SendMessage;
}

String WebSocketConnection::GetCertificateBuffer()
{
    std::ifstream Infile;   
    Infile.open("resources/cacert.pem");
    if (!Infile.is_open())
    {
        return nullptr;
    }

    std::string Line;
    std::string CertString;
    bool bCertReadFlag = false;
    while (std::getline(Infile, Line))
	{
        if (!bCertReadFlag && Line.compare("-----BEGIN CERTIFICATE-----") == 0)
        {
            bCertReadFlag = true;
        }

        if (bCertReadFlag)
        {
            CertString += Line;
            CertString += '\n';
        }


		if (Line.compare("-----END CERTIFICATE-----") == 0)
		{
			break;
		}
    }

	return CertString;
}

SSL_CTX* WebSocketConnection::CreateSSLContext()
{
    SSL_CTX* SslContext = nullptr;
    const SSL_METHOD* SslMethod = SSLv23_client_method();
    if (SslMethod)
    {
        SslContext = SSL_CTX_new(SslMethod);
        if (SslContext)
        {
            // Restrict protocols we do not want
            int RestrictedProtocols = 0;

#define RESTRICT_SSL_TLS_PROTOCOL(EnumVal, OpenSSLBit) RestrictedProtocols |= OpenSSLBit;
            RESTRICT_SSL_TLS_PROTOCOL(SSLv2, SSL_OP_NO_SSLv2);
            RESTRICT_SSL_TLS_PROTOCOL(SSLv3, SSL_OP_NO_SSLv3);
            RESTRICT_SSL_TLS_PROTOCOL(TLSv1, SSL_OP_NO_TLSv1);
            RESTRICT_SSL_TLS_PROTOCOL(TLSv1_1, SSL_OP_NO_TLSv1_1);
            RESTRICT_SSL_TLS_PROTOCOL(TLSv1_2, SSL_OP_NO_TLSv1_2);
#undef RESTRICT_SSL_TLS_PROTOCOL

            int SslContextFlags = 0;
            SslContextFlags |= SSL_OP_NO_COMPRESSION;

            SSL_CTX_set_options(SslContext, SslContextFlags | RestrictedProtocols);

            // Add cert.
            x509_store_st* CertStore = SSL_CTX_get_cert_store(SslContext);
            String CertStrPtr = GetCertificateBuffer();
            BIO* CertificateBio = BIO_new_mem_buf(CertStrPtr.c_str(), CertStrPtr.size());
            X509* CertificatePtr = PEM_read_bio_X509(CertificateBio, NULL, 0, NULL);
            if (CertificatePtr != nullptr)
            {
                X509_STORE_add_cert(CertStore, CertificatePtr);
            }
            else
            {
                uint64 Error = ERR_get_error();
                if (Error != SSL_ERROR_NONE)
                {
                    char AnsiErrorBuffer[256];
                    ERR_error_string_n(Error, AnsiErrorBuffer, sizeof(AnsiErrorBuffer) - 1);
                }
            }
        }
        else
        {
            vvr_log("FSslManager::CreateSslContext: Failed to create the SSL context");
        }
    }
    else
    {
        vvr_log("FSslManager::CreateSslContext: Failed to create method SSLv23_client_method");
    }

    return SslContext;
}