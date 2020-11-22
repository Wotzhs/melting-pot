#ifndef WORKERS_H
#define WORKERS_H

#define NATS_HAS_STREAMING true
#include <nats/nats.h>
#include <iostream>

class Stan {
	natsStatus s;
	stanConnOptions    *connOpts   =   nullptr;
	stanSubOptions     *subOpts    =   nullptr;
	stanConnection     *sc         =   nullptr;
	stanSubscription   *sub        =   nullptr;
	bool               connLost    =   false;
public:
	Stan(natsOptions *opts, const char* cluster, const char* clientID) {
		s = stanConnOptions_Create(&connOpts);
		if (s == NATS_OK) {
			s = stanConnOptions_SetNATSOptions(connOpts, opts);
		}

		if (s == NATS_OK) {
			s = stanConnOptions_SetConnectionLostHandler(connOpts, connectionLostCallBack, &connOpts);
		}

		if (s == NATS_OK) {
			s = stanConnection_Connect(&sc, cluster, clientID, connOpts);
		}

		natsOptions_Destroy(opts);
		stanConnOptions_Destroy(connOpts);
	}

	void Subscribe(const char* durableName, const char* subject) {
		if (s == NATS_OK) {
			s = stanSubOptions_Create(&subOpts);
		}

		if (s == NATS_OK) {
			s = stanSubOptions_SetDurableName(subOpts, durableName);
		}

		if (s == NATS_OK) {
			s = stanSubOptions_StartWithLastReceived(subOpts);
		}

		if (s == NATS_OK) {
			s = stanConnection_Subscribe(&sub, sc, subject, onMessage, NULL, subOpts);
		}
	}

private:
	static void connectionLostCallBack(stanConnection *sc, const char *errText, void *closure) {
		bool *connLost = (bool*) closure;
		std::cout << "connection lost: " << errText << std::endl;
		*connLost = true;
	}

	static void onMessage(stanConnection *sc, stanSubscription *sub, const char *channel, stanMsg *msg, void *closure) {
		printf("Received on [%s]: sequence: %" PRIu64 " data: %.*s timestamp: %" PRId64 " redelivered: %s\n",
			channel,
			stanMsg_GetSequence(msg),
			stanMsg_GetDataLength(msg),
			stanMsg_GetData(msg),
			stanMsg_GetTimestamp(msg),
			stanMsg_IsRedelivered(msg) ? "yes" : "no");
		
		stanMsg_Destroy(msg);
	}
};

#endif
