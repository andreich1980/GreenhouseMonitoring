# Greenhouse monitoring

## Config file

The application uses a config file to read important settings.
The config file should be stored on the SD card in the `/config.json` file.

```json
{
  "hostname": "greenhouse",
  "telegram_bot_token": "XXXXXXXXXX:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
  "chat_id": "XXXXXXXXXX"
}
```

### Configuration options

* `hostname` (string): The host name for the web server. This will be used to access the server from other devices on the same network. The format for accessing the server will be <hostname>.local.
* `telegram_bot_token` (string): The API token for the Telegram bot that will be used for notifications.
* `chat_id` (string): The chat ID for the Telegram chat that will receive notifications.

## Build web interface

```shell
cd www
npm install
npm run build
```

It should build the React web app and put the files in the `/www/dist` folder.

After building, copy the contents of `/www/dist` folder to the SD card in the `/www` folder. You should be able to access the interface at http://greenhouse.local.

## Credits

I received assistance from ChatGPT, an AI language model trained by OpenAI, in developing this project. ChatGPT provided guidance on Arduino and C++ topics and helped me improve my code. I thank ChatGPT for its valuable assistance.
