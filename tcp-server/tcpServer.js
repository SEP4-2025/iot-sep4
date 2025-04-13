const mqtt = require("mqtt");
const client = mqtt.connect("mqtt://localhost:1883");

const PORT = 1337;
const HOST = "0.0.0.0";

const server = net.createServer(
  {
    keepAlive: true,
  },
  (socket) => {
    console.log(
      "Client connected:",
      socket.remoteAddress + ":" + socket.remotePort
    );

    socket.on("data", (data) => {
      console.log("Data received from client:", data.toString());
      socket.write("Server received: " + data.toString());
    });

    socket.on("end", () => {
      console.log("Client disconnected");
    });

    socket.on("error", (err) => {
      console.error("Socket error:", err);
    });
  }
);

server.listen(PORT, HOST, () => {
  console.log("Server listening on " + HOST + ":" + PORT);
});

client.subscribe("mbed NXP LPC1768", (err) => {
  client.on("message", (topic, message) => {
    console.log("Message received in first");
    console.log(topic);
    console.log(message.toString());
  });
});
