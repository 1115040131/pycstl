/*
 Navicat Premium Data Transfer

 Source Server         : llfcmysql
 Source Server Type    : MySQL
 Source Server Version : 80027 (8.0.27)
 Source Host           : 81.68.86.146:3308
 Source Schema         : llfc

 Target Server Type    : MySQL
 Target Server Version : 80027 (8.0.27)
 File Encoding         : 65001

 Date: 03/08/2024 19:53:45
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

CREATE DATABASE IF NOT EXISTS pyc_chat;
USE pyc_chat;

-- ----------------------------
-- Table structure for friend
-- ----------------------------
DROP TABLE IF EXISTS `friend`;
CREATE TABLE `friend`  (
  `id` int UNSIGNED NOT NULL AUTO_INCREMENT,
  `self_id` int NOT NULL,
  `friend_id` int NOT NULL,
  `back` varchar(255) NULL DEFAULT '',
  PRIMARY KEY (`id`) USING BTREE,
  UNIQUE INDEX `self_friend`(`self_id` ASC, `friend_id` ASC) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 1 ROW_FORMAT = Dynamic;

-- ----------------------------
-- Records of friend
-- ----------------------------

-- ----------------------------
-- Table structure for friend_apply
-- ----------------------------
DROP TABLE IF EXISTS `friend_apply`;
CREATE TABLE `friend_apply`  (
  `id` bigint NOT NULL AUTO_INCREMENT,
  `from_uid` int NOT NULL,
  `to_uid` int NOT NULL,
  `status` smallint NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`) USING BTREE,
  UNIQUE INDEX `from_to_uid`(`from_uid` ASC, `to_uid` ASC) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 1 ROW_FORMAT = Dynamic;

-- ----------------------------
-- Records of friend_apply
-- ----------------------------

-- ----------------------------
-- Table structure for user
-- ----------------------------
DROP TABLE IF EXISTS `user`;
CREATE TABLE `user`  (
  `id` int NOT NULL AUTO_INCREMENT,
  `uid` int NOT NULL DEFAULT 0,
  `name` varchar(255) NOT NULL DEFAULT '',
  `email` varchar(255) NOT NULL DEFAULT '',
  `pwd` varchar(255) NOT NULL DEFAULT '',
  `nick` varchar(255) NOT NULL DEFAULT '',
  `desc` varchar(255) NOT NULL DEFAULT '',
  `sex` int NOT NULL DEFAULT 0,
  `icon` varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`) USING BTREE,
  UNIQUE INDEX `uid`(`uid` ASC) USING BTREE,
  UNIQUE INDEX `email`(`email` ASC) USING BTREE,
  INDEX `name`(`name` ASC) USING BTREE
) ENGINE = InnoDB AUTO_INCREMENT = 1 ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Records of user
-- ----------------------------
INSERT INTO `user` VALUES (1, 1, 'pyc', '1115040131@qq.com', '745230', 'pyc', '', 0, '/chat/client/res/head_1.jpg');
INSERT INTO `user` VALUES (2, 2, 'llfc', 'secondtonone1@163.com', '745230', 'llfc', '', 0, '/chat/client/res/head_1.jpg');
INSERT INTO `user` VALUES (3, 3, 'tc', '18165031775@qq.com', '745230', 'tc', '', 0, '/chat/client/res/head_1.jpg');
INSERT INTO `user` VALUES (101, 101, 'test_user_1', 'test_email_1@test.com', '745230', 'test_nick_1', '', 0, '/chat/client/res/head_1.jpg');
INSERT INTO `user` VALUES (102, 102, 'test_user_2', 'test_email_2@test.com', '745230', 'test_nick_2', '', 0, '/chat/client/res/head_1.jpg');
INSERT INTO `user` VALUES (103, 103, 'test_user_3', 'test_email_3@test.com', '745230', 'test_nick_3', '', 0, '/chat/client/res/head_1.jpg');
INSERT INTO `user` VALUES (104, 104, 'test_user_4', 'test_email_4@test.com', '745230', 'test_nick_4', '', 0, '/chat/client/res/head_1.jpg');
INSERT INTO `user` VALUES (105, 105, 'test_user_5', 'test_email_5@test.com', '745230', 'test_nick_5', '', 0, '/chat/client/res/head_1.jpg');
INSERT INTO `user` VALUES (106, 106, 'test_user_6', 'test_email_6@test.com', '745230', 'test_nick_6', '', 0, '/chat/client/res/head_1.jpg');
INSERT INTO `user` VALUES (107, 107, 'test_user_7', 'test_email_7@test.com', '745230', 'test_nick_7', '', 0, '/chat/client/res/head_1.jpg');
INSERT INTO `user` VALUES (108, 108, 'test_user_8', 'test_email_8@test.com', '745230', 'test_nick_8', '', 0, '/chat/client/res/head_1.jpg');
INSERT INTO `user` VALUES (109, 109, 'test_user_9', 'test_email_9@test.com', '745230', 'test_nick_9', '', 0, '/chat/client/res/head_1.jpg');
INSERT INTO `user` VALUES (110, 110, 'test_user_10', 'test_email_10@test.com', '745230', 'test_nick_10', '', 0, '/chat/client/res/head_1.jpg');
INSERT INTO `user` VALUES (111, 111, 'test_user_11', 'test_email_11@test.com', '745230', 'test_nick_11', '', 0, '/chat/client/res/head_1.jpg');
INSERT INTO `user` VALUES (112, 112, 'test_user_12', 'test_email_12@test.com', '745230', 'test_nick_12', '', 0, '/chat/client/res/head_1.jpg');
INSERT INTO `user` VALUES (113, 113, 'test_user_13', 'test_email_13@test.com', '745230', 'test_nick_13', '', 0, '/chat/client/res/head_1.jpg');
INSERT INTO `user` VALUES (114, 114, 'test_user_14', 'test_email_14@test.com', '745230', 'test_nick_14', '', 0, '/chat/client/res/head_1.jpg');
INSERT INTO `user` VALUES (115, 115, 'test_user_15', 'test_email_15@test.com', '745230', 'test_nick_15', '', 0, '/chat/client/res/head_1.jpg');
INSERT INTO `user` VALUES (116, 116, 'test_user_16', 'test_email_16@test.com', '745230', 'test_nick_16', '', 0, '/chat/client/res/head_1.jpg');
INSERT INTO `user` VALUES (117, 117, 'test_user_17', 'test_email_17@test.com', '745230', 'test_nick_17', '', 0, '/chat/client/res/head_1.jpg');
INSERT INTO `user` VALUES (118, 118, 'test_user_18', 'test_email_18@test.com', '745230', 'test_nick_18', '', 0, '/chat/client/res/head_1.jpg');
INSERT INTO `user` VALUES (119, 119, 'test_user_19', 'test_email_19@test.com', '745230', 'test_nick_19', '', 0, '/chat/client/res/head_1.jpg');
INSERT INTO `user` VALUES (120, 120, 'test_user_20', 'test_email_20@test.com', '745230', 'test_nick_20', '', 0, '/chat/client/res/head_1.jpg');

-- ----------------------------
-- Table structure for user_id
-- ----------------------------
DROP TABLE IF EXISTS `user_id`;
CREATE TABLE `user_id`  (
  `id` int NOT NULL,
  PRIMARY KEY (`id`) USING BTREE
) ENGINE = InnoDB ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Records of user_id
-- ----------------------------
INSERT INTO `user_id` VALUES (1000);

-- ----------------------------
-- Procedure structure for reg_user
-- ----------------------------
DROP PROCEDURE IF EXISTS `reg_user`;
delimiter ;;
CREATE PROCEDURE `reg_user`(
    IN `new_name` VARCHAR(255),
    IN `new_email` VARCHAR(255),
    IN `new_pwd` VARCHAR(255),
    OUT `result` INT)
BEGIN
    -- 如果在执行过程中遇到任何错误，则回滚事务
    DECLARE EXIT HANDLER FOR SQLEXCEPTION
    BEGIN
        -- 回滚事务
        ROLLBACK;
        -- 设置返回值为-1，表示错误
        SET result = -1;
    END;
    -- 开始事务
    START TRANSACTION;
    -- 检查用户名是否已存在
    IF EXISTS (SELECT 1 FROM `user` WHERE `name` = new_name) THEN
        SET result = 0; -- 用户名已存在
        COMMIT;
    ELSE
        -- 用户名不存在，检查email是否已存在
        IF EXISTS (SELECT 1 FROM `user` WHERE `email` = new_email) THEN
            SET result = 0; -- email已存在
            COMMIT;
        ELSE
            -- email也不存在，更新user_id表
            UPDATE `user_id` SET `id` = `id` + 1;
            -- 获取更新后的id
            SELECT `id` INTO @new_id FROM `user_id`;
            -- 在user表中插入新记录
            INSERT INTO `user` (`uid`, `name`, `email`, `pwd`) VALUES (@new_id, new_name, new_email, new_pwd);
            -- 设置result为新插入的uid
            SET result = @new_id; -- 插入成功，返回新的uid
            COMMIT;
        END IF;
    END IF;
END
;;
delimiter ;

CALL reg_user('pyc', '1115040131@qq.com', '745230', @result);

SET FOREIGN_KEY_CHECKS = 1;
