#include "chat_service.h"
#include <pqxx/pqxx>
#include <google/protobuf/timestamp.pb.h>

using grpc::Status;
using grpc::StatusCode;

namespace messenger {

ChatServiceImpl::ChatServiceImpl(Database &db, RedisClient &redis): _db(db), _redis(redis) {}

Status ChatServiceImpl::CreateChat(ServerContext* context, const CreateChatRequest* request, CreateChatResponse* response) {
    try {
        pqxx::work w(_db.conn());
        auto res = w.exec_params("INSERT INTO chats (name, is_group) VALUES ($1,$2) RETURNING id", request->name(), request->is_group());
        std::string chat_id = res[0][0].as<std::string>();
        for (const auto &u : request->participant_usernames()) {
            w.exec_params("INSERT INTO chat_members (chat_id, username) VALUES ($1,$2)", chat_id, u);
        }
        w.commit();
        response->set_chat_id(chat_id);
        return Status::OK;
    } catch(const std::exception &e) {
        return Status(StatusCode::INTERNAL, e.what());
    }
}

Status ChatServiceImpl::SendMessage(ServerContext* context, const SendMessageRequest* request, SendMessageResponse* response) {
    try {
        pqxx::work w(_db.conn());
        pqxx::binarystring bs(request->ciphertext());
        auto r = w.exec_params("INSERT INTO messages (chat_id, sender, ciphertext, metadata) VALUES ($1,$2,$3,$4) RETURNING id",
                               request->chat_id(), request->sender(), bs, request->metadata());
        std::string msgid = r[0][0].as<std::string>();
        w.commit();
        // publish via redis for subscribers
        _redis.publish("events:" + request->sender(), std::string("message_sent:" + msgid));
        response->set_ok(true);
        response->set_message_id(msgid);
        return Status::OK;
    } catch(const std::exception &e) {
        return Status(StatusCode::INTERNAL, e.what());
    }
}

Status ChatServiceImpl::GetHistory(ServerContext* context, const GetHistoryRequest* request, GetHistoryResponse* response) {
    try {
        pqxx::work w(_db.conn());
        auto r = w.exec_params("SELECT id, chat_id, sender, ciphertext, metadata, created_at FROM messages WHERE chat_id=$1 ORDER BY created_at DESC LIMIT $2 OFFSET $3",
                               request->chat_id(), request->limit(), request->offset());
        for (auto row : r) {
            messenger::Message *m = response->add_messages();
            m->set_id(row[0].as<std::string>());
            m->set_chat_id(row[1].as<std::string>());
            m->set_sender(row[2].as<std::string>());
            m->set_ciphertext(row[3].as<std::string>());
            m->set_metadata(row[4].as<std::string>());
            google::protobuf::Timestamp *t = new google::protobuf::Timestamp();
            // naive parsing: store as now
            m->mutable_timestamp()->set_seconds(time(NULL));
        }
        return Status::OK;
    } catch(const std::exception &e) {
        return Status(StatusCode::INTERNAL, e.what());
    }
}

}
