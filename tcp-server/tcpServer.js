const mqtt = require("mqtt");
const client = mqtt.connect("mqtt://localhost:1883");

client.subscribe("light:reading", (err) => {
  client.on("message", (topic, message) => {
    console.log(topic);
    console.log(message.toString());
  });
});
