#include "AxisCamera.h"
#include "QtNetwork/qauthenticator.h"
#include "UI/InterThread.hpp"
#include "gloox/disco.h"
#include "gloox/message.h"
#include "gloox/gloox.h"
#include "gloox/siprofileft.h"
#include "gloox/siprofilefthandler.h"
#include "gloox/bytestreamdatahandler.h"
#include "gloox/socks5bytestreamserver.h"
#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>
using namespace EagleLib;

SETUP_PROJECT_IMPL

 
AxisSocket::AxisSocket() 
{
    network_manager = new QNetworkAccessManager(this);
    network_request = nullptr;
    connect(network_manager, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)), this, SLOT(onAuthenticationRequired(QNetworkReply*,QAuthenticator*)));
}

void AxisSocket::request(QUrl url)
{
	qDebug() << "Sending Request: " << url; 
    network_request = network_manager->get(QNetworkRequest(url));
    connect(network_request, SIGNAL(finished()), this, SLOT(requestFinished()));
    connect(network_request, SIGNAL(readyRead()), this, SLOT(requestReadyRead()));
    connect(network_request, SIGNAL(downloadProgress(quint64,quint64)), this, SLOT(requestDownloadProgress(quint64,quint64)));
} 

void AxisSocket::requestReadyRead()
{
    qDebug() << "Request ready to read";
	QByteArray line = network_request->readLine();
	
	while (line.size())
	{
		std::string str = QString(line).toStdString();
		int idx = str.find('=');
		std::string first = str.substr(0, idx);
		std::string second = str.substr(idx + 1, str.size() - 3 - first.size());
		std::cout << first << " " << second << std::endl;
		if (first == "zoom")
		{
			zoom->UpdateData(boost::lexical_cast<int>(second));
		}
		if (first == "focus")
		{
			focus->UpdateData(boost::lexical_cast<int>(second));
		}
		line = network_request->readLine();
	}
	
}

void AxisSocket::requestFinished()
{

}

void AxisSocket::requestDownloadProgress(quint64 received, quint64 total)
{

}

void AxisSocket::onAuthenticationRequired(QNetworkReply* reply, QAuthenticator* auth)
{
    //qDebug() << "Authentication required";
	auth->setUser(username);
	auth->setPassword(password);
}

void AxisCamera::on_panRequest()
{
    int port = *getParameter<int>("Camera port")->Data();
    int tilt = *getParameter<int>("Camera pan")->Data();
	std::string userName = *getParameter<std::string>("Camera username")->Data();
	std::string pass = *getParameter<std::string>("Camera password")->Data();
    int cameraNum = *getParameter<int>("Camera number")->Data();
	std::string portStr;
	if (port != -1)
		portStr = boost::lexical_cast<std::string>(port);
	std::string tiltStr = (tilt < 0) ? "lpan=" : "rpan=";
	std::string requestStr = "http://";
	if (userName.size() && pass.size())
		requestStr += userName + ":" + pass + "@";
	requestStr += *getParameter<std::string>("Camera address")->Data() +
		portStr + "/axis-cgi/ptz.cgi?" +
		tiltStr + boost::lexical_cast<std::string>(abs(tilt)) +
		"&camera=" + boost::lexical_cast<std::string>(cameraNum);

    NODE_LOG(info) << "Pan request: " << requestStr;
    socket->request(QUrl(QString::fromStdString(requestStr)));
    updateParameter("Camera moving", true);
}
void AxisCamera::on_zoomRequest()
{ 
    int port = *getParameter<int>("Camera port")->Data();
    int tilt = *getParameter<int>("Camera zoom")->Data();
    int cameraNum = *getParameter<int>("Camera number")->Data();
	std::string portStr;
	if (port != -1)
		portStr = boost::lexical_cast<std::string>(port);
	std::string tiltStr = "zoom=";
	std::string requestStr = "http://" + *getParameter<std::string>("Camera address")->Data() +
		portStr + "/axis-cgi/com/ptz.cgi?" +
		tiltStr + boost::lexical_cast<std::string>(abs(tilt)) +
		"&camera=" + boost::lexical_cast<std::string>(cameraNum);

    NODE_LOG(info) << "Zoom request: " << requestStr;
    socket->request(QUrl(QString::fromStdString(requestStr)));
    updateParameter("Camera moving", true);
}
void AxisCamera::on_tiltRequest() 
{
    int port = *getParameter<int>("Camera port")->Data();
    int tilt = *getParameter<int>("Camera tilt")->Data();
    int cameraNum = *getParameter<int>("Camera number")->Data();
	std::string portStr;
	if (port != -1)
		portStr = boost::lexical_cast<std::string>(port);
	std::string tiltStr = (tilt < 0) ? "ltilt=" : "rtilt=";
    std::string requestStr = "http://" + *getParameter<std::string>("Camera address")->Data() +
            portStr + "/axis-cgi/ptz.cgi?" +
            tiltStr + boost::lexical_cast<std::string>(abs(tilt)) + 
			"&camera=" + boost::lexical_cast<std::string>(cameraNum);

    NODE_LOG(info) << "Tilt request: " << requestStr;
    socket->request(QUrl(QString::fromStdString(requestStr)));
    updateParameter("Camera moving", true);
	on_credentialChange();
}
void AxisCamera::get_position()
{
	QString url = "http://" + QString::fromStdString(*getParameter<std::string>("Camera address")->Data()) + "/axis-cgi/com/ptz.cgi?query=position";
	socket->request(QUrl(url));
}
void AxisCamera::on_addressChange()
{

}
void AxisCamera::on_credentialChange()
{
	socket->username = QString::fromStdString(*getParameter<std::string>("Camera username")->Data());
	socket->password = QString::fromStdString(*getParameter<std::string>("Camera password")->Data());
}
void AxisCamera::Serialize(ISimpleSerializer* pSerializer)
{
	Node::Serialize(pSerializer); 
	SERIALIZE(socket);
}
void AxisCamera::Init(bool firstInit)
{
	if (firstInit) 
	{
		socket.reset(new AxisSocket());

        updateParameter<std::string>("Camera address", "192.168.1.152");
		updateParameter("Camera port", int(-1));
        updateParameter<std::string>("Camera username", "root"); 
        updateParameter<std::string>("Camera password", "12369pp"); // TODO change to password string

		updateParameter("Camera calibration file", Parameters::ReadFile("Camera_Calibration.yml"));
		
		updateParameter("Camera zoom", int(0));
		updateParameter("Camera pan", int(0));
		updateParameter("Camera tilt", int(0));
        updateParameter("Camera number", int(1));

        updateParameter("Camera moving", false, Parameters::Parameter::State);

		currentZoom.reset(new Parameters::TypedParameter<int>("Current Zoom", 0, Parameters::Parameter::State));
		currentFocus.reset(new Parameters::TypedParameter<int>("Current Focus", 0, Parameters::Parameter::State));
		socket->zoom = currentZoom;
		socket->focus = currentFocus;
		parameters.push_back(currentZoom);
		parameters.push_back(currentFocus);

		updateParameter("Camera matrix", cv::Mat(), Parameters::Parameter::Output);
		updateParameter("Camera pose", cv::Mat(), Parameters::Parameter::Output);
	}
	else
	{
		currentZoom = std::dynamic_pointer_cast<Parameters::TypedParameter<int>>(getParameter<int>("Current Zoom"));
		currentFocus = std::dynamic_pointer_cast<Parameters::TypedParameter<int>>(getParameter<int>("Current Focus"));
	}
	updateParameter<boost::function<void(void)>>("Get position", boost::bind(&AxisCamera::get_position, this));
    zoomConnection = getParameter("Camera zoom")->RegisterNotifier(boost::bind(&AxisCamera::on_zoomRequest, this));
    panConnection = getParameter("Camera pan")->RegisterNotifier(boost::bind(&AxisCamera::on_panRequest, this));
    tiltConnection = getParameter("Camera tilt")->RegisterNotifier(boost::bind(&AxisCamera::on_tiltRequest, this));
	ipConnection = getParameter("Camera address")->RegisterNotifier(boost::bind(&AxisCamera::on_addressChange, this));
	usernameConnection = getParameter("Camera username")->RegisterNotifier(boost::bind(&AxisCamera::on_credentialChange, this));
	passwordConnection = getParameter("Camera password")->RegisterNotifier(boost::bind(&AxisCamera::on_credentialChange, this));
	on_credentialChange();
}

AxisCamera::~AxisCamera()
{
	zoomConnection.disconnect(); 
	panConnection.disconnect();
	tiltConnection.disconnect();
	ipConnection.disconnect();
	usernameConnection.disconnect();
	passwordConnection.disconnect();
}

cv::cuda::GpuMat AxisCamera::doProcess(cv::cuda::GpuMat& img, cv::cuda::Stream& stream)
{
	return img;
}
bool AxisCamera::SkipEmpty() const 
{
	return false;
}


using namespace gloox;

void XmppClient::onConnect()
{
	LOG_TRACE;
}
void XmppClient::onDisconnect(ConnectionError e)
{
	LOG_TRACE;
}
bool XmppClient::onTLSConnect(const CertInfo& info)
{
	LOG_TRACE;
	return true;
}
void XmppClient::handleMessage(const Message& msg, MessageSession * session)
{
	LOG_TRACE;
	auto body = msg.body();
	NODE_LOG(debug) << "Received message " << body;
	updateParameter<std::string>("Message", body);
	auto nodes = getNodesInScope();
	
    if (boost::starts_with(body, "SetParameter"))
    {
        std::stringstream ss(body);
		std::string line;
		std::getline(ss, line);
		while (ss.good())
		{
			try
			{
                auto inputParam = Parameters::Persistence::Text::DeSerialize(&ss);
				for (auto node : nodes)
				{
					if (node->fullTreeName == inputParam->GetTreeRoot())
					{
						for (auto param : node->parameters)
						{
							if (param->GetName() == inputParam->GetName())
							{
								param->Update(inputParam);
							}

						}
					}
					
				}
			}
			catch (...)
			{

			}			
		}
    }
    if(boost::starts_with(body, "ListParameters"))
    {
        sendParameters(session);
    }
}
void XmppClient::sendParameters(MessageSession* session)
{
	std::stringstream ss;
	auto nodes = getNodesInScope();
	for (auto node : nodes)
	{
		for (auto param : node->parameters)
		{
            Parameters::Persistence::Text::Serialize(&ss, param.get());
		}
	}
	session->send(ss.str());
}

void XmppClient::handleMessageEvent(const JID& from, MessageEventType messageEvent)
{
	LOG_TRACE;
}
void XmppClient::handleChatState(const JID& from, ChatStateType state)
{
	LOG_TRACE;
}
void XmppClient::handleMessageSession(MessageSession *session)
{
	LOG_TRACE;
	xmpp_client->disposeMessageSession(m_session);
	m_session = session;
	m_session->registerMessageHandler(this);
	m_messageEventFilter = new MessageEventFilter(m_session);
	m_messageEventFilter->registerMessageEventHandler(this);
	m_chatStateFilter = new ChatStateFilter(m_session);
	m_chatStateFilter->registerChatStateHandler(this);
	m_session->send(*getParameter<std::string>("Public IP")->Data());
    sendParameters(m_session);

}
void XmppClient::handleLog(LogLevel level, LogArea area, const std::string& message)
{
	LOG_TRACE;
	switch (level)
	{
	case LogLevelDebug:
	{
		NODE_LOG(debug) << message;
	}
	case LogLevelError:
	{
		NODE_LOG(error) << message;
	}
	case LogLevelWarning:
	{
		NODE_LOG(warning) << message;
	}
		
	}
}

void XmppClient::on_msgReceived(std::string& msg)
{
	
}

void XmppClient::Init(bool firstInit)
{
	if (firstInit)
	{
		m_session = nullptr;
		updateParameter<std::string>("Jabber id", "dtmoodie");
		updateParameter<std::string>("Password", "12369pp");
		updateParameter<std::string>("Jabber server", "jabber.iitsp.com");
		updateParameter<unsigned short>("Server port", 5222);
		addInputParameter<cv::cuda::GpuMat>("Input point cloud");
		RegisterParameterCallback("Input point cloud", boost::bind(&XmppClient::_sendPointCloud, this));
                updateParameter<std::string>("Public IP", "68.100.56.64");
	}
	
}
void XmppClient::_sendPointCloud()
{
	Parameters::UI::ProcessingThreadCallbackService::Instance()->post(boost::bind(&XmppClient::sendPointCloud, this));
}
void XmppClient::sendPointCloud()
{
	auto gpuMat = getParameter<cv::cuda::GpuMat>("Input point cloud")->Data();
	if (gpuMat && m_session && gpuMat->rows)
	{
/*		std::string message;
		message.resize(4 + gpuMat->rows * sizeof(float) * 3);
		cv::Mat h_mat(gpuMat->rows, gpuMat->cols, CV_32F, (void*)(message.data() + 4));
		gpuMat->download(h_mat);
		memcpy((void*)message.data(), (void*)&gpuMat->rows, sizeof(int));
		m_session->send(message);*/
	}
}
cv::cuda::GpuMat XmppClient::doProcess(cv::cuda::GpuMat& img, cv::cuda::Stream& stream)
{
	if (parameters[0]->changed || parameters[1]->changed || parameters[2]->changed || parameters[3]->changed)
	{
		gloox::JID jid(*getParameter<std::string>(0)->Data() + "@" + *getParameter<std::string>(2)->Data());;
		xmpp_client.reset(new gloox::Client(jid, *getParameter<std::string>(1)->Data(), *getParameter<unsigned short>(3)->Data()));
		xmpp_client->registerConnectionListener(this);
		xmpp_client->registerMessageSessionHandler(this, 0);
		xmpp_client->disco()->setVersion("messageTest", GLOOX_VERSION, "Linux");
		xmpp_client->disco()->setIdentity("client", "bot");
		xmpp_client->disco()->addFeature(XMLNS_CHAT_STATES);
		xmpp_client->logInstance().registerLogHandler(LogLevelDebug, LogAreaAll, this);
		if (!xmpp_client->connect(false))
		{
			NODE_LOG(error) << "Unable to connect";
		}
		parameters[0]->changed = false;
		parameters[1]->changed = false;
		parameters[2]->changed = false;
		parameters[3]->changed = false;
	}
	if (xmpp_client)
	{
		xmpp_client->recv(0);
	}
	return img;
}

NODE_DEFAULT_CONSTRUCTOR_IMPL(AxisCamera)
NODE_DEFAULT_CONSTRUCTOR_IMPL(XmppClient)

/*


*  Copyright (c) 2004-2015 by Jakob Schr\F6ter <js@camaya.net>
*  This file is part of the gloox library. http://camaya.net/gloox
*
*  This software is distributed under a license. The full license
*  agreement can be found in the file LICENSE in this distribution.
*  This software may not be copied, modified, sold or distributed
*  other than expressed in the named license agreement.
*
*  This software is distributed without any warranty.


#include "../client.h"
#include "../messagesessionhandler.h"
#include "../messageeventhandler.h"
#include "../messageeventfilter.h"
#include "../chatstatehandler.h"
#include "../chatstatefilter.h"
#include "../connectionlistener.h"
#include "../disco.h"
#include "../message.h"
#include "../gloox.h"
#include "../lastactivity.h"
#include "../loghandler.h"
#include "../logsink.h"
#include "../connectiontcpclient.h"
#include "../connectionsocks5proxy.h"
#include "../connectionhttpproxy.h"
#include "../messagehandler.h"
using namespace gloox;

#ifndef _WIN32
# include <unistd.h>
#endif

#include <stdio.h>
#include <string>

#include <cstdio> // [s]print[f]

#if defined( WIN32 ) || defined( _WIN32 )
# include <windows.h>
#endif

class MessageTest : public MessageSessionHandler, ConnectionListener, LogHandler,
	MessageEventHandler, MessageHandler, ChatStateHandler
{
public:
	MessageTest() : m_session(0), m_messageEventFilter(0), m_chatStateFilter(0) {}

	virtual ~MessageTest() {}

	void start()
	{

		JID jid("hurkhurk@example.net/gloox");
		j = new Client(jid, "hurkhurks");
		j->registerConnectionListener(this);
		j->registerMessageSessionHandler(this, 0);
		j->disco()->setVersion("messageTest", GLOOX_VERSION, "Linux");
		j->disco()->setIdentity("client", "bot");
		j->disco()->addFeature(XMLNS_CHAT_STATES);
		StringList ca;
		ca.push_back("/path/to/cacert.crt");
		j->setCACerts(ca);

		j->logInstance().registerLogHandler(LogLevelDebug, LogAreaAll, this);

		//
		// this code connects to a jabber server through a SOCKS5 proxy
		//
		//       ConnectionSOCKS5Proxy* conn = new ConnectionSOCKS5Proxy( j,
		//                                   new ConnectionTCP( j->logInstance(),
		//                                                      "sockshost", 1080 ),
		//                                   j->logInstance(), "example.net" );
		//       conn->setProxyAuth( "socksuser", "sockspwd" );
		//       j->setConnectionImpl( conn );

		//
		// this code connects to a jabber server through a HTTP proxy through a SOCKS5 proxy
		//
		//       ConnectionTCP* conn0 = new ConnectionTCP( j->logInstance(), "old", 1080 );
		//       ConnectionSOCKS5Proxy* conn1 = new ConnectionSOCKS5Proxy( conn0, j->logInstance(), "old", 8080 );
		//       conn1->setProxyAuth( "socksuser", "sockspwd" );
		//       ConnectionHTTPProxy* conn2 = new ConnectionHTTPProxy( j, conn1, j->logInstance(), "jabber.cc" );
		//       conn2->setProxyAuth( "httpuser", "httppwd" );
		//       j->setConnectionImpl( conn2 );


		if (j->connect(false))
		{
			ConnectionError ce = ConnNoError;
			while (ce == ConnNoError)
			{
				ce = j->recv();
			}
			printf("ce: %d\n", ce);
		}

		delete(j);
	}

	virtual void onConnect()
	{
		printf("connected!!!\n");
	}

	virtual void onDisconnect(ConnectionError e)
	{
		printf("message_test: disconnected: %d\n", e);
		if (e == ConnAuthenticationFailed)
			printf("auth failed. reason: %d\n", j->authError());
	}

	virtual bool onTLSConnect(const CertInfo& info)
	{
		time_t from(info.date_from);
		time_t to(info.date_to);

		printf("status: %d\nissuer: %s\npeer: %s\nprotocol: %s\nmac: %s\ncipher: %s\ncompression: %s\n"
			"from: %s\nto: %s\n",
			info.status, info.issuer.c_str(), info.server.c_str(),
			info.protocol.c_str(), info.mac.c_str(), info.cipher.c_str(),
			info.compression.c_str(), ctime(&from), ctime(&to));
		return true;
	}

	virtual void handleMessage(const Message& msg, MessageSession * session)
	{
		printf("type: %d, subject: %s, message: %s, thread id: %s\n", msg.subtype(),
			msg.subject().c_str(), msg.body().c_str(), msg.thread().c_str());

		std::string re = "You said:\n> " + msg.body() + "\nI like that statement.";
		std::string sub;
		if (!msg.subject().empty())
			sub = "Re: " + msg.subject();

		m_messageEventFilter->raiseMessageEvent(MessageEventDisplayed);
#if defined( WIN32 ) || defined( _WIN32 )
		Sleep(1000);
#else
		sleep(1);
#endif
		m_messageEventFilter->raiseMessageEvent(MessageEventComposing);
		m_chatStateFilter->setChatState(ChatStateComposing);
#if defined( WIN32 ) || defined( _WIN32 )
		Sleep(2000);
#else
		sleep(2);
#endif
		m_session->send(re, sub);

		if (msg.body() == "quit")
			j->disconnect();
	}

	virtual void handleMessageEvent(const JID& from, MessageEventType event)
	{
		printf("received event: %d from: %s\n", event, from.full().c_str());
	}

	virtual void handleChatState(const JID& from, ChatStateType state)
	{
		printf("received state: %d from: %s\n", state, from.full().c_str());
	}

	virtual void handleMessageSession(MessageSession *session)
	{
		printf("got new session\n");
		// this example can handle only one session. so we get rid of the old session
		j->disposeMessageSession(m_session);
		m_session = session;
		m_session->registerMessageHandler(this);
		m_messageEventFilter = new MessageEventFilter(m_session);
		m_messageEventFilter->registerMessageEventHandler(this);
		m_chatStateFilter = new ChatStateFilter(m_session);
		m_chatStateFilter->registerChatStateHandler(this);
	}

	virtual void handleLog(LogLevel level, LogArea area, const std::string& message)
	{
		printf("log: level: %d, area: %d, %s\n", level, area, message.c_str());
	}

private:
	Client *j;
	MessageSession *m_session;
	MessageEventFilter *m_messageEventFilter;
	ChatStateFilter *m_chatStateFilter;
};

int main(int argc, char** argv)
{
	MessageTest *r = new MessageTest();
	r->start();
	delete(r);
	return 0;
}


*/
