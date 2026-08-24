# 🎮 GameTextTranslator

> 游戏内文本翻译工具 — 截图即翻译，轻松玩转外服

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)
[![Qt](https://img.shields.io/badge/Qt-6.8.3-brightgreen.svg)](https://www.qt.io/)
[![Platform](https://img.shields.io/badge/platform-Windows-blue.svg)]()

---

## 📖 简介

GameTextTranslator 是一款专为游戏玩家设计的文本翻译工具。通过快捷键截取屏幕区域，自动识别并翻译游戏内的对话、聊天文本，帮助玩家轻松应对韩服、美服等外服游戏的语言障碍。

**核心流程：** 截图 → OCR 识别 → 自动翻译 → 悬浮窗显示

---

## ✨ 功能特点

- 🔥 **一键截图翻译**：按下热键（默认 `Ctrl+Alt+T`），拖拽选择区域，自动翻译
- 🎯 **精准选区**：鼠标拖拽选择任意区域，支持 4K/2K/1080p 屏幕
- 🌐 **多语言识别**：支持中文、英文、日文、韩文等多语言 OCR 识别
- 🔄 **自动翻译**：集成百度翻译 API，自动识别源语言并翻译
- 🖼️ **智能悬浮窗**：翻译结果自动适配文本长度，不遮挡游戏画面
- ⚙️ **可视化配置**：图形界面配置 API 密钥、热键、开机自启等
- 🚀 **开机自启**：可选开机自动启动，随系统运行
- 📦 **轻量便携**：基于 Qt 6 + C++ 开发，资源占用低

---

## 📸 截图

> 待补充

---

## 🚀 快速开始

### 下载

从 [Releases](https://github.com/你的用户名/GameTextTranslator/releases) 下载最新版本。

### 安装

1. 解压压缩包到任意目录
2. 运行 `GameTextTranslator.exe`

### 使用

1. 启动程序（后台运行，在系统托盘中显示图标）
2. 右键托盘图标 → 配置 → 填写百度翻译 API 密钥
3. 在游戏中按下截图热键（默认 `Ctrl+Alt+T`）
4. 鼠标拖拽选中需要翻译的文本区域
5. 翻译结果自动在右下角显示

---

## 🔧 配置说明

### 百度翻译 API 申请

1. 访问 [百度翻译开放平台](https://fanyi-api.baidu.com/)
2. 注册/登录 → 开通「通用翻译」服务
3. 获取 `APP ID` 和 `密钥`
4. 在软件「配置」界面中填写

> 免费版每月 100 万字符，个人使用完全够用。

### 配置文件

配置界面支持「保存」「另存为」「加载」功能，用户可以：

- 将配置保存到任意路径的 JSON 文件
- 加载之前保存的配置文件
- 管理多套配置（如不同游戏使用不同热键/语言）

### 配置文件结构

```json
{
    "baidu_app_id": "你的APP_ID",
    "baidu_secret_key": "你的密钥",
    "hotkey": "Ctrl+Alt+T",
    "auto_start": false,
    "ocr_language": "config_chinese.txt",
    "target_language": "zh"
}
```