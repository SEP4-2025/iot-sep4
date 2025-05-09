const mqtt = require("mqtt");
const client = mqtt.connect("mqtt://34.27.128.90:1883");

// .on connect -> waits until the connection is actually established
client.on("connect", () => {
  // client.publish("pump/command", "start", { qos: 1, retain: true }, (err) => {
  //   if (err) {
  //     console.error("Failed to publish message:", err);
  //   } else {
  //     console.log(`Sent 'start' command to 'pump/command'`);
  //   }
  // });

  client.subscribe(
    {
      "light/reading": { qos: 0 },
      "soil/reading": { qos: 0 },
      "air/temperature": { qos: 0 },
      "air/humidity": { qos: 0 },
      "pump/started": { qos: 0 },
      "pump/stopped": { qos: 0 },
    },
    (err) => {
      if (err) {
        console.error("Failed to subscribe:", err);
      } else {
        console.log("Subscribed to all relevant topics");
      }
    },
  );

  setTimeout(() => {
    // client.publish("pump/command", "5000", { qos: 1}, (err) => {
    //   if (err) {
    //     console.error("Failed to publish message:", err);
    //   } else {
    //     console.log(`Sent 'start' command to 'pump/command'`);
    //   }
    // });
    // setTimeout(() => {
    //   client.publish("pump/command_stop", "force stop the pump", { qos: 1}, (err) => {
    //     if (err) {
    //       console.error("Failed to publish force stop pump message:", err);
    //     } else {
    //       console.log(`Sent 'force stop' command to 'pump/command'`);
    //     }
    //   });
    // }, 3000)
    // setTimeout(() => {
    //   client.publish("pump/command", "5000", { qos: 1}, (err) => {
    //     if (err) {
    //       console.error("Failed to publish message:", err);
    //     } else {
    //       console.log(`Sent 'start' command to 'pump/command'`);
    //     }
    //   });
    // }, 6000)

    client.publish(
      "pump/command_start",
      "start the pump",
      { qos: 1 },
      (err) => {
        if (err) {
          console.error("Failed to publish message:", err);
        } else {
          console.log("sent pump command start");
        }
      },
    );

    setTimeout(() => {
      client.publish(
        "pump/command_stop",
        "force stop the pump",
        { qos: 1 },
        (err) => {
          if (err) {
            console.error("Failed to publish force stop pump message:", err);
          } else {
            console.log(`Sent 'force stop' command to 'pump/command'`);
          }
        },
      );
    }, 5000);
  }, 20000);

  // setTimeout(() => {
  //   let count = 0;
  //   const interval = setInterval(() => {
  //     if (count >= 5) {
  //       clearInterval(interval);
  //       console.log("Finished sending 5 messages.");
  //       return;
  //     }

  //     const message = "start"
  //     client.publish("pump/command", message, { qos: 1, retain: true }, (err) => {
  //       if (err) {
  //         console.error(`Failed to publish message ${count + 1}:`, err);
  //       } else {
  //         console.log(`Published '${message}' to 'pump/command'`);
  //       }
  //     });

  //     count++;
  //   }, 1000); // Send every 1 second
  // }, 20000);

  client.on("message", (topic, message) => {
    const msg = message.toString();

    if (topic === "light/reading") {
      console.log("Light level:", msg);
    } else if (topic === "pump/started") {
      console.log("Pump started confirmation:", msg);
    } else if (topic === "pump/stopped") {
      console.log("Pump stopped confirmation:", msg);
    } else if (topic === "air/temperature") {
      console.log("Air temperature:", msg);
    } else if (topic === "air/humidity") {
      console.log("Air humidity:", msg);
    } else if (topic === "soil/reading") {
      console.log("Soil moisture level:", msg);
    } else {
      console.log("Unknown topic:", topic, msg);
    }
  });
});
