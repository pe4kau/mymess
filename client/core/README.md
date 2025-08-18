# messenger_client_backend (C++20, gRPC, TLS)

Клиентский сетевой модуль для мессенджера. Содержит реализацию INetworkClient, менеджеры (Auth/Chat/Event/Key), CryptoEngine (упрощённый Double Ratchet на базе OpenSSL: X25519 + HKDF-SHA256 + AES-256-GCM), и SessionStore (SQLite/SQLCipher).

## Сборка
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build
```

## Зависимости
- gRPC, Protobuf
- OpenSSL
- SQLite3 (если SQLCipher, то PRAGMA key будет работать автоматически)

## Как использовать
- Создайте `NetworkClient` (см. src/NetworkClient.cpp) и передайте его в менеджеры.
- Используйте `KeyManager` для публикации pre-keys и получения бандла собеседника.
- При первом сообщении:
  1) Получите `PreKeyBundle` другого пользователя через `GetPreKeyBundleAsync`.
  2) Вызовите `CryptoEngine::InitOutbound(...)`, зашифруйте сообщение `Encrypt(...)`.
  3) Отправьте через `ChatManager::SendEncrypted(...)` (внутри `INetworkClient::SendMessageAsync`).

## Примечания
- Файлы protobuf/gRPC генерируются в `${build}/generated`.
- CryptoEngine реализует минимум, достаточный для тестов и обмена сообщениями.
