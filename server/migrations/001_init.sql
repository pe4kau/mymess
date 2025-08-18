-- init schema for messenger
CREATE TABLE IF NOT EXISTS users (
  id uuid PRIMARY KEY DEFAULT gen_random_uuid(),
  username TEXT UNIQUE NOT NULL,
  password_hash bytea NOT NULL,
  identity_key bytea,
  created_at timestamptz DEFAULT now()
);

CREATE TABLE IF NOT EXISTS device_prekeys (
  id uuid PRIMARY KEY DEFAULT gen_random_uuid(),
  username TEXT NOT NULL,
  device_id TEXT NOT NULL,
  pre_key bytea,
  identity_key bytea,
  updated_at timestamptz DEFAULT now(),
  UNIQUE(username, device_id)
);

CREATE TABLE IF NOT EXISTS chats (
  id uuid PRIMARY KEY DEFAULT gen_random_uuid(),
  name TEXT,
  is_group boolean DEFAULT false,
  created_at timestamptz DEFAULT now()
);

CREATE TABLE IF NOT EXISTS chat_members (
  id uuid PRIMARY KEY DEFAULT gen_random_uuid(),
  chat_id uuid REFERENCES chats(id) ON DELETE CASCADE,
  username TEXT NOT NULL
);

CREATE TABLE IF NOT EXISTS messages (
  id uuid PRIMARY KEY DEFAULT gen_random_uuid(),
  chat_id uuid REFERENCES chats(id) ON DELETE CASCADE,
  sender TEXT NOT NULL,
  ciphertext bytea NOT NULL,
  metadata TEXT,
  created_at timestamptz DEFAULT now()
);

CREATE INDEX IF NOT EXISTS idx_messages_chat ON messages(chat_id, created_at);
