const { log, info, warn, error } = require("console");
const express = require("express");
const app = express();
const http = require("http");
const server = http.createServer(app);
const { Server } = require("socket.io");
const io = new Server(server, {
  allowEIO3: true,
  cors: {
    origin: "*",
    methods: ["GET", "POST"],
    transports: ["websocket", "polling"],
    credentials: true,
  },
});

app.use(express.static("public"));

app.get("/", (req, res) => {
  req.redirect("/air");
});

app.get("/air", (req, res) => {
  res.sendFile(__dirname + "/public/index.html");
});

app.get("/soil", (req, res) => {
  res.sendFile(__dirname + "/public/index2.html");
});

io.on("connection", (socket) => {
  info(
    "[" + socket.id + "] new connection",
    socket.request.connection.remoteAddress
  );

  socket.on("message", (data) => {
    log(`message from ${data.clientID} via socket id: ${socket.id}`);
    io.emit("message", data);
  });
  socket.on("message1", (data) => {
    log(`message from ${data.clientID} via socket id: ${socket.id}`);
    io.emit("message1", data);
  });

  socket.on("UPDATE", (data) => {
    log(`request update for person ${data}`);
    io.emit("UPDATE", data);
  });

  /**them gi thi them o giua day */
  socket.on("event_name", (data) => {
    log(data)
  })
  /**************************** */
  //xu ly chung
  socket.on("reconnect", function () {
    warn("[" + socket.id + "] reconnect.");
  });
  socket.on("disconnect", () => {
    error("[" + socket.id + "] disconnect.");
  });
  socket.on("connect_error", (err) => {
    error(err.stack);
  });
});

//doi port khac di
server.listen(3000, () => {
  log("server is listening on port doi port di");
});

