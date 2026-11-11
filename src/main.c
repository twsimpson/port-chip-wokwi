// For information and examples see:
// https://link.wokwi.com/custom-chips-alpha

#include "wokwi-api.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  pin_t sel[2];
  pin_t port_a[8];
  pin_t port_b[8];
  pin_t port_p[8];
} chip_state_t;

static inline void bus_init(pin_t* pins, char prefix)
{
  char pin_name[3] = { prefix, '0', '\0' };
  for (uint32_t index = 0; index < 8; ++index)
  {
    pin_name[1] = (char)((uint8_t)'0' + index);
    pins[index] = pin_init(pin_name, INPUT);
  }
}

static void chip_sel_pin_change(void *user_data, pin_t pin, uint32_t value);
static void chip_port_pin_change(void *user_data, pin_t pin, uint32_t value);

void chip_init(void) {
  chip_state_t *chip = malloc(sizeof(chip_state_t));

  chip->sel[0] = pin_init("SEL0", INPUT);
  chip->sel[1] = pin_init("SEL1", INPUT);
  bus_init(chip->port_a, 'A');
  bus_init(chip->port_b, 'B');
  bus_init(chip->port_p, 'P');

  const pin_watch_config_t sel_pin_watch_config = {
    .user_data = chip,
    .edge = BOTH,
    .pin_change = &chip_sel_pin_change
  };
  pin_watch(chip->sel[0], &sel_pin_watch_config);
  pin_watch(chip->sel[1], &sel_pin_watch_config);
  chip_sel_pin_change(chip, 0, 0);
  puts("Online!");
}

static inline uint8_t bus_read(const pin_t* pins)
{
  uint8_t result = 0;
  for (uint32_t index = 0; index < 8; ++index)
  {
    if (pin_read(pins[index]) != LOW)
    {
      result |= (1 << index);
    }
  }
  return result;
}

static inline void bus_write(const pin_t* pins, uint8_t value)
{
  for (uint32_t index = 0; index < 8; ++index)
  {
    pin_write(pins[index], ((value >> index) & 1) == 0 ? LOW : HIGH);
  }
}

static inline void bus_mode_input(const pin_t* pins)
{
  for (uint32_t index = 0; index < 8; ++index)
  {
    pin_mode(pins[index], INPUT);
  }
}

static inline void bus_mode_output(const pin_t* pins, uint8_t value)
{
  for (uint32_t index = 0; index < 8; ++index)
  {
    pin_mode(pins[index], ((value >> index) & 1) == 0 ? OUTPUT_LOW : OUTPUT_HIGH);
  }
}

static inline void bus_watch(const pin_t* pins, const pin_watch_config_t* pin_watch_config)
{
  for (uint32_t index = 0; index < 8; ++index)
  {
    pin_watch(pins[index], pin_watch_config);
  }
}

static inline void bus_watch_stop(const pin_t* pins)
{
  for (uint32_t index = 0; index < 8; ++index)
  {
    pin_watch_stop(pins[index]);
  }
}

void chip_sel_pin_change(void *user_data, pin_t pin, uint32_t value)
{
  printf("chip_sel_pin_change(chip=%p, pin=%i, value=%u)\n", user_data, pin, value);
  chip_state_t* chip = (chip_state_t*)user_data;
  const pin_watch_config_t bus_pin_watch_config = {
    .user_data = user_data,
    .edge = BOTH,
    .pin_change = &chip_port_pin_change
  };
  if (pin_read(chip->sel[0]) != LOW)
  {
    if (pin_read(chip->sel[1]) != LOW)
    {
      // 1 1 = P -> B
      bus_watch_stop(chip->port_a);
      bus_mode_input(chip->port_a);
      bus_watch_stop(chip->port_b);
      bus_mode_input(chip->port_p);
      bus_mode_output(chip->port_b, bus_read(chip->port_p));
      bus_watch(chip->port_p, &bus_pin_watch_config);
      puts("P -> B");
    }
    else
    {
      // 0 1 = B -> P
      bus_watch_stop(chip->port_a);
      bus_mode_input(chip->port_a);
      bus_watch_stop(chip->port_p);
      bus_mode_input(chip->port_b);
      bus_mode_output(chip->port_p, bus_read(chip->port_b));
      bus_watch(chip->port_b, &bus_pin_watch_config);
      puts("B -> P");
    }
  }
  else if (pin_read(chip->sel[1]) != LOW)
  {
    // 1 0 = P -> A
    bus_watch_stop(chip->port_a);
    bus_watch_stop(chip->port_b);
    bus_mode_input(chip->port_p);
    bus_mode_output(chip->port_a, bus_read(chip->port_p));
    bus_watch(chip->port_p, &bus_pin_watch_config);
    puts("P -> A");
  }
  else
  {
    // 0 0 = A -> P
    bus_watch_stop(chip->port_b);
    bus_mode_input(chip->port_b);
    bus_watch_stop(chip->port_p);
    bus_mode_input(chip->port_b);
    bus_mode_output(chip->port_p, bus_read(chip->port_a));
    bus_watch(chip->port_a, &bus_pin_watch_config);
    puts("A -> P");
  }
}

void chip_port_pin_change(void *user_data, pin_t pin, uint32_t value)
{
  chip_state_t* chip = (chip_state_t*)user_data;
  if (pin_read(chip->sel[0]) != LOW)
  {
    if (pin_read(chip->sel[1]) != LOW)
    {
      // 1 1 = P -> B
      bus_write(chip->port_b, bus_read(chip->port_p));
    }
    else
    {
      // 0 1 = B -> P
      bus_write(chip->port_p, bus_read(chip->port_b));
    }
  }
  else if (pin_read(chip->sel[1]) != LOW)
  {
    // 1 0 = P -> A
    bus_write(chip->port_a, bus_read(chip->port_p));
  }
  else
  {
    // 0 0 = A -> P
    bus_write(chip->port_p, bus_read(chip->port_a));
  }
}
