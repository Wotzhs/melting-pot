import { Request, Response } from "express";
import { Controller, Methods } from "./decorators";
import { AccountService } from "./services";

@Controller("/")
export class SignUpControlller {
  private accountService: AccountService;

  constructor() {
    this.accountService = new AccountService();
  }

  @Methods.Get("signup")
  async signup(req: Request, res: Response) {
    const [resp, err] = await this.accountService.CreateAccount(req);
    if (err) {
      return res.status(400).send(err);
    }

    res.send(resp);
  }
}