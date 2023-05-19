export class ConsoleLogger {
  log(message, location) {
    console.warn(`${location}: ${message}`);
  }
}

export let fallbackLogger = new ConsoleLogger();

/// Captures log messages, and also forwards them to a base logger.
export class CapturingLogger {
  #baseLogger;
  #messages = [];

  constructor(baseLogger) {
    this.#baseLogger = baseLogger ?? fallbackLogger;
  }

  get didLogMessage() {
    return this.#messages.length > 0;
  }
  get loggedMessages() {
    return this.#messages;
  }

  getLoggedMessagesStringForToolTip() {
    return this.#messages.join("\n");
  }

  log(message, location) {
    this.#messages.push(new CapturedLogMessage(message, location));
    this.#baseLogger.log(message, location);
  }
}

export class CapturedLogMessage {
  location;
  message;

  constructor(message, location) {
    this.location = location;
    this.message = message;
  }

  toString() {
    return `${this.message} (${this.location})`;
  }
}
