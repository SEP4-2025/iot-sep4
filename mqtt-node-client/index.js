const mqtt = require("mqtt");
const client = mqtt.connect("mqtt://localhost:1883");

// .on connect -> waits until the connection is actually established    
client.on("connect", () => {
  client.publish("pump:command", "start", { qos: 1, retain: true }, (err) => {
    if (err) {
      console.error("Failed to publish message:", err);
    } else {
      console.log(`Sent 'start' command to 'pump:command'`);
    }
  });

  client.subscribe(
    {
      "light:reading": { qos: 0 },
      "pump:started": { qos: 0 },
      "pump:stopped": { qos: 0 }
    },
    (err) => {
      if (err) {
        console.error("Failed to subscribe:", err);
      } else {
        console.log("Subscribed to all relevant topics");
      }
    }
  );

  client.on("message", (topic, message) => {
    const msg = message.toString();
    console.log("entered in client on message")
  
    if (topic === "light:reading") {
      console.log("Light level:", msg);
    } else if (topic === "pump:started") {
      console.log("Pump started confirmation:", msg);
    } else if (topic === "pump:stopped") {
      console.log("Pump stopped confirmation:", msg);
    } else {
      console.log("Unknown topic:", topic, msg);
    }
  });
});

