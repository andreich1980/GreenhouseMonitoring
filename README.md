# Greenhouse tracking

## Config file

The application uses a config file to read important settings.
The config file should be stored on the LittleFS filesystem in the `/config.json` file.

```json
{
  "telegram_bot_token": "XXXXXXXXXX:XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX",
  "telegram_chat_id": "XXXXXXXXXX",
  "temperature_min": 16,
  "temperature_max": 30,
  "thingspeak_channel_id": 1111111,
  "thingspeak_write_api_key": "XXXXXXXXXXXXXXXX"
}
```

### Configuration options

* `telegram_bot_token` (string): The API token for the Telegram bot that will be used for notifications.
* `telegram_chat_id` (string): The chat ID for the Telegram chat that will receive notifications.
Use [@myidbot](https://t.me/myidbot) to find out the chat ID of an individual or a group.
* `temperature_min`, `temperature_max` (integer): When the temperature goes out of these boundaries, the bot will send a notification.
* `thingspeak_channel_id` (integer): Channel ID on [ThingSpeak.com](https://thingspeak.com).
* `thingspeak_write_api_key` (integer): Write API Key for the channel.

Copy an example config file from `data/config.example.js` to `data/config.js` and fill in your Telegram bot API token, ThingSpeak channel ID and write API key, as well as other settings.
Upload the file into the filesystem.

## Credits

I received assistance from ChatGPT, an AI language model trained by OpenAI, in developing this project. ChatGPT provided guidance on Arduino and C++ topics and helped me improve my code. I thank ChatGPT for its valuable assistance.
