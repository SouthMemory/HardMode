DROP TABLE IF EXISTS `hardmode_rewards_ondeath`;
CREATE TABLE IF NOT EXISTS `hardmode_rewards_ondeath` (
  `mode` int NOT NULL,
  `death_count` int NOT NULL,
  `reward_type` int NOT NULL,
  `reward_id` int NOT NULL,
  `reward_count` int DEFAULT NULL,
  `comment` varchar(50) COLLATE utf8mb4_general_ci DEFAULT NULL,
  PRIMARY KEY (`mode`,`death_count`,`reward_type`,`reward_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;




-- 动态生成插入语句
-- Mode 4 数据
DELIMITER $$

DROP PROCEDURE IF EXISTS generate_hardmode_rewards$$

CREATE PROCEDURE generate_hardmode_rewards()
BEGIN
    DECLARE level INT DEFAULT 1;
    DECLARE death_count INT DEFAULT 1;

    -- 插入 mode = 4 的数据, 升级奖励
    WHILE level <= 100 DO
        REPLACE INTO `hardmode_rewards` (`mode`, `reward_level`, `reward_type`, `reward_id`, `reward_count`, `comment`)
        VALUES (4, level, 0, 60000, level * 3, CONCAT('Level ', level, ' reward x 3'));
        SET level = level + 1;
    END WHILE;

    SET level = 1;

    -- 插入 mode = 3 的数据, 升级奖励
    WHILE level <= 100 DO
        REPLACE INTO `hardmode_rewards` (`mode`, `reward_level`, `reward_type`, `reward_id`, `reward_count`, `comment`)
        VALUES (3, level, 0, 60000, level * 5, CONCAT('Level ', level, ' reward x 5'));
        SET level = level + 1;
    END WHILE;

    -- 插入 mode = 4 的数据，死亡奖励
    WHILE death_count <= 108 DO
        REPLACE INTO `hardmode_rewards_ondeath` (`mode`, `death_count`, `reward_type`, `reward_id`, `reward_count`, `comment`)
        VALUES (4, death_count, 0, 60000, death_count * 2, CONCAT('Death ', death_count, ' reward x 2'));
        SET death_count = death_count + 1;
    END WHILE;

END$$

DELIMITER ;

-- 调用存储过程生成数据
CALL generate_hardmode_rewards();
