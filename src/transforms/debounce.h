#ifndef debounce_H
#define debounce_H

#include "transforms/transform.h"

static const char DEBOUNCE_SCHEMA[] PROGMEM = R"###({
    "type": "object",
    "properties": {
        "min_delay": { "title": "Minimum delay", "type": "number", "description": "The minimum time in ms between inputs for output to happen" }
    }
  })###";

/**
 * @brief Implements debounce code for a button or switch
 *
 * It's a passthrough transform that will output a value only if it's different
 * from the previous output, and only if it's been ms_min_delay ms since the
 * input was received, with no other input received since then.
 *
 * @tparam T The type of value being passed through Debounce.
 *
 * @param ms_min_delay The minimum amount of time that must have passed since
 * the input was received by this Transform in order for the output to occur. If
 * you are using this to debounce the output from `DigitalInputChange`,
 * ms_min_delay should be set at least a little bit longer than
 * `DigitalInputChange::read_delay`.
 *
 * DigitalInputChange::read_delay is 10 ms by default, and
 * Debounce::ms_min_delay is 15 ms by default. If that doesn't adequately
 * debounce your button or switch, adjust both of those values until it does.
 *
 * @param config_path The path for configuring ms_min_delay with the Config UI.
 * @see DigitalInputChange
 */
template <class T>
class DebounceTemplate : public SymmetricTransform<T> {
 public:
  DebounceTemplate(int ms_min_delay = 15, String config_path = "")
      : SymmetricTransform<T>(config_path),
        ms_min_delay_{ms_min_delay} {
    this->load_configuration();
  }

  virtual void set_input(T input, uint8_t input_channel = 0) override {
    if (reaction_ != NULL) {
      reaction_->remove();
      reaction_ = nullptr;
    }
    // Input has changed since the last emit, or this is the first
    // input since the program started to run.
    if (input != stable_input_ || value_sent_ == false) {
      reaction_ = app.onDelay(ms_min_delay_, [this, input]() {
        this->reaction_ = nullptr;
        this->stable_input_ = input;
        this->value_sent_ = true;
        this->emit(input);
      });
    }
  }

 private:
  int ms_min_delay_;
  bool value_sent_ = false;
  T stable_input_;
  DelayReaction* reaction_ = nullptr;
  virtual void get_configuration(JsonObject& doc) override {
    doc["min_delay"] = ms_min_delay_;
  }

  virtual bool set_configuration(const JsonObject& config) override {
    String expected[] = {"min_delay"};
    for (auto str : expected) {
      if (!config.containsKey(str)) {
        return false;
      }
    }
    ms_min_delay_ = config["min_delay"];
    return true;
  }

  virtual String get_config_schema() override { return FPSTR(DEBOUNCE_SCHEMA); }
};

typedef DebounceTemplate<bool> DebounceBool;
typedef DebounceTemplate<bool>
    Debounce;  // for backward-compatibility with original class
typedef DebounceTemplate<int> DebounceInt;
typedef DebounceTemplate<float>
    DebounceFloat;  // not sure this works - test it if you use it

#endif
