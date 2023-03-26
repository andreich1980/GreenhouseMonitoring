# Greenhouse monitoring

## Config file

The application uses a config file to read important settings.
The config file should be stored on the SD card in the `/config.json` file.

```json
{
  "hostname": "greenhouse",
  "telegram_bot_token": "XXXXXXXXXX:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
  "telegram_chat_id": "XXXXXXXXXX",
  "temperature_min": 16,
  "temperature_max": 30,
  "thingspeak_channel_id": 1111111,
  "thingspeak_write_api_key": "XXXXXXXXXXXXXXXX"
}
```

### Configuration options

* `hostname` (string): The host name for the web server. This will be used to access the server from other devices on the same network. The format for accessing the server will be <hostname>.local.
* `telegram_bot_token` (string): The API token for the Telegram bot that will be used for notifications.
* `telegram_chat_id` (string): The chat ID for the Telegram chat that will receive notifications.
Use [@myidbot](https://t.me/myidbot) to find out the chat ID of an individual or a group.
* `temperature_min`, `temperature_max` (integer): When the temperature goes out of these boundaries, the bot will send a notification.
* `thingspeak_channel_id` (integer): Channel ID on [ThingSpeak.com](https://thingspeak.com).
* `thingspeak_write_api_key` (integer): Write API Key for the channel.

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
