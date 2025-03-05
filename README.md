[![Build Status](https://gpu-audio.teamcity.com/app/rest/builds/affectedProject:(id:ContainerPlugin_Gpu_FirProcessor)/statusIcon)](https://gpu-audio.teamcity.com/project.html?projectId=ContainerPlugin_Gpu_FirProcessor&tab=projectOverview)

# FIR Processor

## Host Code Components

### FirModule
Implements the interface for the engine to create and destroy the processor.

### FirDeviceCodeProvider
Provides the engine access to the device binary, which is embedded in the processor binary.

### FirModuleInfoProvider
Implements the interfaces to query properties of the processor like name, id, version and supported GPU platforms.
Also provides the engines with the names of the processor device functions.

### FirModuleLibrary
Defines the module export functions to create and destroy the FirModule, FirDeviceCodeProvider and FirModuleInfoProvider.

### FirProcessor
This is the host-side of the processor and implements the processor interface. Configures the execution of the processor
and provides parameters for the GPU tasks.

### ImpulseResponseStore
Provides methods to load impulse response .wav audio files in memory.

## Device Code Components

### Properties
Defines the names for device function substitution to avoid name conflicts between processors.
Also contains user-defined parameter structs, which are passed to the processor functions during processing.

### FirProcessor.cuh
The device side implementation of the processor. Defines the GPU processor and its tasks, i.e., the processing functions.

### FirProcessor.cu
Declares the GPU tasks and the GPU processor using pre-defined macros.
