#ifndef FRONIUS_TYPES_H_
#define FRONIUS_TYPES_H_

/**
 * @brief Container for project enums and their string conversion helpers.
 *
 * This struct holds all related enums and provides static
 * constexpr `toString()` functions to convert enum values to
 * human-readable strings, useful for logging, error messages,
 * and debugging.
 */
struct FroniusTypes {
  /**
   * @brief Represents AC or DC phases.
   */
  enum class Phase {
    TOTAL,   /**< Total phase */
    AVERAGE, /**< Average phase */
    A,       /**< Phase A */
    B,       /**< Phase B */
    C        /**< Phase C */
  };

  /**
   * @brief Get a string representation of a Phase enum.
   *
   * @param phase The Phase value to convert.
   * @return A C-style string representing the Phase value.
   */
  static constexpr const char *toString(Phase phase) {
    switch (phase) {
    case Phase::TOTAL:
      return "TOTAL";
    case Phase::AVERAGE:
      return "AVERAGE";
    case Phase::A:
      return "A";
    case Phase::B:
      return "B";
    case Phase::C:
      return "C";
    default:
      return "UNKNOWN";
    }
  }

  // -------- Input --------

  /**
   * @brief Represents inverter input channels.
   */
  enum class Input {
    TOTAL, /**< Total input */
    A,     /**< Input A */
    B      /**< Input B */
  };

  /**
   * @brief Get a string representation of an Input enum.
   *
   * @param input The Input value to convert.
   * @return A C-style string representing the Input value.
   */
  static constexpr const char *toString(Input input) {
    switch (input) {
    case Input::TOTAL:
      return "TOTAL";
    case Input::A:
      return "A";
    case Input::B:
      return "B";
    default:
      return "UNKNOWN";
    }
  }
};

#endif /* FRONIUS_TYPES_H_ */
