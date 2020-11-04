import "reflect-metadata";
import express from "express";
import { serve, setup } from "swagger-ui-express";
import Router from "./router";
import DB from "./db";
import swaggerSpec from "./swagger.json";

export class App {
  private app: express.Application;
  private readonly port = 5000;

  constructor() {
    this.app = express();
    this.setupSwagger();
    Router.register(this.app);
    DB.connect();
  }

  setupSwagger() {
    this.app.use("/", serve, setup(swaggerSpec));
  }

  run() {
    this.app.listen(this.port, () =>
      console.log(`Running on port ${this.port}`)
    );
  }
}
