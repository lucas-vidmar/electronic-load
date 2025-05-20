<script type="module">
  import mermaid from "https://cdn.jsdelivr.net/npm/mermaid@latest/dist/mermaid.esm.min.mjs"
  import elkLayouts from "https://cdn.jsdelivr.net/npm/@mermaid-js/layout-elk@latest/dist/mermaid-layout-elk.esm.min.mjs"

  // register ELK
  mermaid.registerLayoutLoaders(elkLayouts)

  // then render the diagrams as usual
  mermaid.initialize({
    startOnLoad: true,
    theme: "default",
  })
</script>
# Hardware

## Overview

This document describes the hardware design of the electronic load project. The electronic load is a device that can sink current from a power source in a controlled manner, useful for testing power supplies, batteries, and other sources.

## Main Components

- **Microcontroller**: Controls the operation, user interface, and feedback loop.
- **Power MOSFET**: Acts as the main variable load element.
- **Current Sense Resistor**: Measures the current flowing through the load.
- **Operational Amplifier**: Provides feedback and control for the MOSFET.
- **Display/Interface**: Shows measurements and allows user input.
- **Cooling System**: Heatsink and/or fan to dissipate heat from the MOSFET.

## Block Diagram

<pre class="mermaid">
  %%{ init: { "flowchart": { "defaultRenderer": "elk" } } }%%

  flowchart TB
      Clock["Clock"] --> PowerSupply["PowerSupply"]
      EncoderInput["EncoderInput"] --> µC["µC"]
      PowerSupply --> µC & Ventilation["Ventilation"]
      µC --> Display["Display"] & ADC["ADC"] & DAC["DAC"]
      Ventilation --> TemperatureSensing["TemperatureSensing"]
      ADC --> VoltageSensing["VoltageSensing"] & CurrentSensing["CurrentSensing"]
      VoltageSensing --> DUT["DUT"] & DUT
      CurrentSensing --> PowerStage["PowerStage"] & DUT
      DAC --> ControlLoop["ControlLoop"]
      ControlLoop --> PowerStage
      PowerStage --> DUT
      TemperatureSensing --> DUT
</pre>

## Schematic

![Schematic](./img/schematic.svg)

## PCB Design

![PCB](./img/pcb.svg)