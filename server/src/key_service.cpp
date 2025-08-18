#include "key_service.h"
#include <pqxx/pqxx>

namespace messenger {

KeyServiceImpl::KeyServiceImpl(Database &db): _db(db) {}

grpc::Status KeyServiceImpl::UploadPreKey(grpc::ServerContext* context, const messenger::UploadPreKeyRequest* request,
                                          messenger::UploadPreKeyResponse* response) {
    try {
        pqxx::work w(_db.conn());
        w.exec_params("INSERT INTO device_prekeys (username, device_id, pre_key, identity_key) VALUES ($1,$2,$3,$4) ON CONFLICT (username,device_id) DO UPDATE SET pre_key = $3, identity_key = $4",
                      request->username(), request->device_id(),
                      pqxx::binarystring(request->pre_key()),
                      pqxx::binarystring(request->identity_key()));
        w.commit();
        response->set_ok(true);
        return grpc::Status::OK;
    } catch (const std::exception &e) {
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

grpc::Status KeyServiceImpl::GetPreKey(grpc::ServerContext* context, const messenger::GetPreKeyRequest* request,
                                       messenger::GetPreKeyResponse* response) {
    try {
        pqxx::work w(_db.conn());
        pqxx::result r = w.exec_params("SELECT pre_key, identity_key FROM device_prekeys WHERE username=$1 LIMIT 1", request->username());
        if (r.empty()) return grpc::Status(grpc::StatusCode::NOT_FOUND, "no prekey");
        std::string pk = r[0][0].as<std::string>();
        std::string ik = r[0][1].as<std::string>();
        response->set_pre_key(pk);
        response->set_identity_key(ik);
        return grpc::Status::OK;
    } catch (const std::exception &e) {
        return grpc::Status(grpc::StatusCode::INTERNAL, e.what());
    }
}

}
