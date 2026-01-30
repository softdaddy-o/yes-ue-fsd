# Action Recording and Playback System

## Overview

The Action Recording and Playback System allows you to record player actions and replay them automatically. This is useful for:
- Creating reusable automation sequences
- Regression testing with recorded gameplay
- Demo recordings for presentations
- Automated stress testing

## Components

### 1. UActionTimeline

Stores timestamped actions with metadata and serialization support.

**Key Features:**
- Add movement, rotation, and input actions
- Export/import to JSON format
- Save/load to disk
- Optimize and compress timelines
- Query actions by time range

**Example Usage:**
```cpp
// Create a timeline
UActionTimeline* Timeline = NewObject<UActionTimeline>();
Timeline->SetRecordingInfo("My Recording", "A test recording");

// Add actions
FAutoDriverMoveParams MoveParams;
MoveParams.TargetLocation = FVector(1000, 0, 100);
Timeline->AddMovementAction(0.5f, FVector(1000, 0, 100), MoveParams);

// Save to file
Timeline->SaveToFile("Saved/Recordings/MyRecording.json");
```

### 2. UActionRecorder

Component that records actions during gameplay.

**Key Features:**
- Start/stop/pause recording
- Automatic movement and rotation tracking
- Configurable recording intervals and thresholds
- Buffer size limits
- Export recordings to files

**Setup:**
1. Add ActionRecorder component to your PlayerController or Pawn
2. Configure recording settings in the Details panel
3. Call `StartRecording()` to begin

**Blueprint Example:**
```
// Start recording
Action Recorder->Start Recording("Test Recording")

// Record for some time...

// Stop and save
Action Recorder->Stop Recording()
Action Recorder->Save Recording("Saved/Recordings/Test.json")
```

**Recording Settings:**
- **Max Recording Duration**: Auto-stop after N seconds (0 = unlimited)
- **Recording Buffer Size**: Maximum actions to keep in memory (default: 10000)
- **Recording Interval**: Minimum time between recorded actions (default: 0.1s)
- **Movement Threshold**: Minimum distance to record movement (default: 10cm)
- **Rotation Threshold**: Minimum angle to record rotation (default: 1°)

### 3. UActionPlayback

Component that plays back recorded timelines.

**Key Features:**
- Play/pause/stop/restart controls
- Variable playback speed
- Looping modes (once, loop, loop count)
- Seek to specific time
- AutoDriver integration

**Setup:**
1. Add ActionPlayback component to your PlayerController or Pawn
2. Ensure an AutoDriverComponent is also present (or set manually)
3. Load and play a timeline

**Blueprint Example:**
```
// Load and play a recording
Action Playback->Load And Play Timeline("Saved/Recordings/Test.json")

// Control playback
Action Playback->Set Playback Speed(2.0)  // 2x speed
Action Playback->Set Playback Mode(Loop)
Action Playback->Pause()
Action Playback->Resume()
```

**Playback Modes:**
- **Once**: Play once and stop
- **Loop**: Loop continuously
- **Loop Count**: Loop a specific number of times

## File Format

Recordings are saved as JSON files with the following structure:

```json
{
  "Metadata": {
    "RecordingName": "Test Recording",
    "Description": "A test recording",
    "MapName": "TestMap",
    "CreatedAt": "2026-01-30 12:00:00",
    "Duration": 10.5,
    "ActionCount": 42,
    "Tags": ["test", "movement"]
  },
  "Actions": [
    {
      "Timestamp": 0.5,
      "ActionType": "Movement",
      "ActionName": "MoveToLocation",
      "ActionData": "{\"X\":1000,\"Y\":0,\"Z\":100,...}"
    },
    {
      "Timestamp": 1.0,
      "ActionType": "Input",
      "ActionName": "Jump",
      "ActionData": "{\"ActionName\":\"Jump\",\"Value\":1.0,...}"
    }
  ]
}
```

## C++ Usage Examples

### Recording Actions

```cpp
// Create and setup recorder
UActionRecorder* Recorder = NewObject<UActionRecorder>(PlayerController);
Recorder->RegisterComponent();

// Configure settings
Recorder->SetMaxDuration(60.0f);  // 60 seconds max
Recorder->SetBufferSize(5000);
Recorder->SetRecordingInterval(0.05f);  // 20 recordings/sec

// Start recording
Recorder->StartRecording("My Test Recording");

// Recording happens automatically...

// Stop and save
Recorder->StopRecording();
Recorder->SaveRecording(FPaths::ProjectSavedDir() / "Recordings" / "MyTest.json");
```

### Playing Back Recordings

```cpp
// Create and setup playback
UActionPlayback* Playback = NewObject<UActionPlayback>(PlayerController);
Playback->RegisterComponent();
Playback->SetAutoDriver(AutoDriverComponent);

// Configure playback
Playback->SetPlaybackSpeed(1.5f);  // 1.5x speed
Playback->SetPlaybackMode(EPlaybackMode::Loop);

// Load and play
FString RecordingPath = FPaths::ProjectSavedDir() / "Recordings" / "MyTest.json";
Playback->LoadAndPlayTimeline(RecordingPath);

// Bind to events
Playback->OnPlaybackFinished.AddDynamic(this, &AMyActor::OnPlaybackComplete);
Playback->OnActionExecuted.AddDynamic(this, &AMyActor::OnActionExecuted);
```

### Manual Action Recording

```cpp
// Add specific actions manually
UActionTimeline* Timeline = NewObject<UActionTimeline>();

// Record movement
FAutoDriverMoveParams MoveParams;
MoveParams.TargetLocation = FVector(1000, 500, 100);
MoveParams.AcceptanceRadius = 50.0f;
MoveParams.SpeedMultiplier = 1.0f;
Timeline->AddMovementAction(1.0f, FVector(1000, 500, 100), MoveParams);

// Record rotation
FAutoDriverRotateParams RotateParams;
RotateParams.TargetRotation = FRotator(0, 90, 0);
RotateParams.RotationSpeed = 180.0f;
Timeline->AddRotationAction(2.0f, FRotator(0, 90, 0), RotateParams);

// Record input
Timeline->AddInputAction(3.0f, "Jump", 1.0f, 0.1f);

// Save timeline
Timeline->SaveToFile("MyCustomRecording.json");
```

## Events and Delegates

### ActionRecorder Events

- **OnRecordingStateChanged**: Fires when recording state changes (recording, paused, stopped)
- **OnActionRecorded**: Fires each time an action is recorded

### ActionPlayback Events

- **OnPlaybackStateChanged**: Fires when playback state changes
- **OnActionExecuted**: Fires each time an action is executed during playback
- **OnPlaybackFinished**: Fires when playback completes
- **OnPlaybackLoopCompleted**: Fires each time a loop iteration completes

## Best Practices

1. **Recording Duration**: Set a max duration to prevent unbounded recordings
2. **Buffer Size**: Adjust based on expected recording length and action frequency
3. **Recording Interval**: Lower intervals = more detail but larger files
4. **Thresholds**: Increase movement/rotation thresholds to reduce noise
5. **Compression**: Call `OptimizeTimeline()` before saving to remove duplicates
6. **File Organization**: Store recordings in `Saved/Recordings/` directory
7. **Testing**: Use `Once` mode for initial testing, then `Loop` for stress testing

## Integration with AutoDriver

The recording system integrates seamlessly with AutoDriver:

```cpp
// Record AutoDriver commands
AutoDriverComponent->MoveToLocation(FVector(1000, 0, 0));
// ActionRecorder automatically captures this

// Playback uses AutoDriver to execute
Playback->Play(Timeline);
// Actions are executed through AutoDriverComponent
```

## Performance Considerations

- **Recording**: Minimal overhead (~0.1ms per recording interval)
- **Playback**: Depends on action complexity and AutoDriver commands
- **Memory**: ~200 bytes per action on average
- **File Size**: JSON format, ~150 bytes per action on disk

## Troubleshooting

**Recording not working:**
- Ensure ActionRecorder component is registered and active
- Check that recording has been started with `StartRecording()`
- Verify thresholds aren't too high (preventing recording)

**Playback not working:**
- Ensure AutoDriverComponent is present and set
- Verify timeline loaded successfully
- Check that playback state is `Playing`
- Ensure actions are compatible with current map/setup

**Actions not executing correctly:**
- Verify playback speed is reasonable (0.5-2.0)
- Check time tolerance setting (default 0.05s)
- Ensure AutoDriver commands are completing successfully

## Future Enhancements

Planned features for future releases:
- Editor UI for browsing and managing recordings
- Visual timeline editor
- Action markers and annotations
- Branching and conditional playback
- Recording compression improvements
- Blueprint recording support
