CREATE TABLE IF NOT EXISTS `hardmode_modes` (
  `id` tinyint unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(50) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci DEFAULT NULL,
  `description` varchar(50) CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci DEFAULT NULL,
  `restrictions` bigint unsigned DEFAULT NULL,
  `enabled` tinyint DEFAULT NULL,
  `maxlives` tinyint unsigned DEFAULT '1',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=7 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_general_ci;

DELETE FROM `hardmode_modes` WHERE `id` IN (3, 4);

REPLACE INTO `acore_world`.`hardmode_modes` (`id`, `name`, `description`, `restrictions`, `enabled`, `maxlives`) VALUES (3, '|cffff0000一命模式|r', '死亡即永恒', 256, 1, 1);
REPLACE INTO `acore_world`.`hardmode_modes` (`id`, `name`, `description`, `restrictions`, `enabled`, `maxlives`) VALUES (4, '|cffff0000水浒模式|r', '梁山好汉再次下凡', 256, 1, 108);