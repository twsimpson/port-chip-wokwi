# Wokwi Port Chip Example

This is a port chip for [Wokwi](https://wokwi.com/). It implements a simple bi-directional port.

## Pin names

| Name        | Description  |
| ----------- | ------------ |
| A0..A7      | Port A       |
| B0..B7      | Port B       |
| P0..P7      | Port P       |
| SEL0, SEL1  | Port Select  |

## Select Mapping
| SEL1 | SEL0 | Description       |
| ---- | ---- | ----------------- |
| LOW  | LOW  | Port A -> Port P  |
| LOW  | HIGH | Port B -> Port P  |
| HIGH | LOW  | Port P -> Port A  |
| HIGH | HIGH | Port P -> Port B  |

## Usage

To use this chip in your project, include it as a dependency in your `diagram.json` file:

```json
  "dependencies": {
    "chip-inverter": "github:twsimpson/port-chip-wokwi@main"
  }
```

Then, add the chip to your circuit by adding a `chip-port` item to the `parts` section of diagram.json:

```json
  "parts": {
    ...,
    { "type": "chip-port", "id": "chip1" }
  },
```

## TODO
For a complete example, see [the inverter chip test project](https://wokwi.com/projects/350946636543820370).
