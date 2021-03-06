#ifndef STAN_H
#define STAN_H

#define NATS_HAS_STREAMING true
#include <nats/nats.h>
#include <iostream>
#include "../workers.hpp"

namespace clients {

class Stan {
	natsStatus s;
	stanConnOptions*  connOpts = nullptr;
	stanSubOptions*   subOpts  = nullptr;
	stanConnection*   sc       = nullptr;
	stanSubscription* sub      = nullptr;
	bool              connLost = false;
	natsOptions*      opts     = nullptr;
	const char*       cluster  = nullptr;
	const char*       clientID = nullptr;
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
			this->opts = opts;
			this->cluster = cluster;
			this->clientID = clientID;
		}

		natsOptions_Destroy(opts);
		stanConnOptions_Destroy(connOpts);
	}

	~Stan() {
		stanSubscription_Close(sub);
		stanSubscription_Destroy(sub);
		stanConnection_Destroy(sc);
		nats_Close();
	}

	Stan(const Stan& stan2): Stan(stan2.opts, stan2.cluster, stan2.clientID) {}

	Stan& operator=(const Stan& stan2) {
		if (this == &stan2) return *this;
		this->opts = stan2.opts;
		this->cluster =  stan2.cluster;
		this->clientID = stan2.clientID;
		return *this;
	}

	Stan(Stan&& stan2) noexcept {
		this->opts = std::exchange(stan2.opts, nullptr);
		this->cluster = std::exchange(stan2.cluster, nullptr);
		this->clientID = std::exchange(stan2.clientID, nullptr);
	}

	Stan& operator=(Stan&& stan2) noexcept {
		std::swap(this->opts, stan2.opts);
		std::swap(this->cluster, stan2.cluster);
		std::swap(this->clientID, stan2.clientID);
		return *this;
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

		stanSubOptions_Destroy(subOpts);
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
			stanMsg_IsRedelivered(msg) ? "yes" : "no"
		);

		if (std::string(channel) == "wallet_created") {
			workers::HandleWalletCreated(stanMsg_GetData(msg));
		}

		stanMsg_Destroy(msg);
	}
};

}

#endif
