# Little Renderer

## Windows

### Requirements
- Visual Studio 2019
- VulkanSDK (with VULKAN_SDK as a system environment)

To generate Visual studio project solution, run

```
> GenerateProject.bat
```

To compile glsl shaders, run

```
> CompileShaders.bat
```

To compile shader info ( mali offline compiler v7.5 ) for each glsl shaders, run

```
> CompileShaders_sc.bat ( compile shader and shader info sametime )
> CompileShaders_onlysc.bat ( compile only shader info, spv shaders are need to be compiled )
```

## Android
### Requirements
- Android Studio
- VulkanSDK (with VULKAN_SDK as a system environment)

### Runnable apk
You can find latest built apks(debug, release) under /apk/

### Benchmark record
Record will be stored under external storage
```
/sdcard/ubm_record_YYYYMMDD_HHMMSS.txt
/sdcard/ubm_record_YYYYMMDD_HHMMSS.onlyNum
/sdcard/ubm_record_YYYYMMDD_HHMMSS.json
```


