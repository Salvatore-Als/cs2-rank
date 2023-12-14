-- Rename table
RENAME TABLE cs2_rank_users TO cs2_rank_stats;

-- Add user_id to stats column
ALTER TABLE cs2_rank_stats ADD COLUMN user_id INT(11) AFTER id;

-- Create new users table
CREATE TABLE cs2_rank_users (
  id bigint NOT NULL AUTO_INCREMENT,
  authid bigint NOT NULL DEFAULT '0',
  name varchar(32) COLLATE utf8mb4_general_ci NOT NULL,
  ignore_annouce int NOT NULL DEFAULT '0',
  lastconnect int NOT NULL DEFAULT '0',
  UNIQUE KEY id (id) USING BTREE
) ENGINE=InnoDB AUTO_INCREMENT=28 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

-- Insert users into the new user table
INSERT INTO cs2_rank_users (authid, name, ignore_annouce, lastconnect)
SELECT authid, MAX(name) AS name, MAX(ignore_annouce) AS ignore_annouce, MAX(lastconnect) AS lastconnect
FROM cs2_rank_stats
GROUP BY authid;

-- Update user_id from all users
UPDATE cs2_rank_stats
JOIN cs2_rank_users ON cs2_rank_stats.authid = cs2_rank_users.authid
SET cs2_rank_stats.user_id = cs2_rank_users.id;

-- Remove useless columns
ALTER TABLE cs2_rank_stats
DROP COLUMN lastconnect,
DROP COLUMN name,
DROP COLUMN ignore_annouce,
DROP COLUMN authid;