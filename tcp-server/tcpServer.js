const mqtt = require("mqtt");
const client = mqtt.connect("mqtt://localhost:1883");

client.on("connect", () => {
  client.subscribe("presence", (err) => {
    if (!err) {
      client.publish("presence", "Hello mqtt");
    }
  });
});

client.subscribe("mbed NXP LPC1768", (err) => {
  client.on("message", (topic, message) => {
    console.log("Message received in first");
    console.log(topic);
    console.log(message.toString());
  });
});
