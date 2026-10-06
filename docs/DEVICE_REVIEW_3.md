# Audio distortion and paragraph alignment — 00.06

The user reported that the 00.05 music sounded distorted and crackled, and that paragraph lines had uneven right edges. Successful compilation and a non-silent PCM check were insufficient to establish audio quality on the Vita.

The output worker now alternates two 64-byte-aligned stereo buffers instead of immediately rewriting the buffer just submitted to the device. It supplies 2048 frames per block, increases file buffering and gives the audio worker priority just ahead of the main thread. The original recording is converted to 48 kHz stereo signed 16-bit little-endian PCM, preserving its channel separation. A portable mixer applies a shared integer gain ramp to both channels, retains frame order, clips mixed output safely and zeroes missing samples. Tests check distinct left/right samples, exact gain, fade completion and missing reads. This fixes the buffer ownership defect and improves the output reserve; audible resolution still requires hardware confirmation.

References: [SDL Vita audio buffer rotation](https://github.com/libsdl-org/SDL/blob/SDL2/src/audio/vita/SDL_vitaaudio.c), [VitaSDK output formats and frame counts](https://docs.vitasdk.org/group__SceAudioUser.html).

Paragraph lines that wrap at word boundaries are now justified across the available content width. Drawing distributes the remaining width across spaces while using the same apostrophe-aware metrics as pagination. Headings, explicit line breaks, final paragraph lines and single-word lines remain aligned left. Tests check wrapping flags and the resulting right edge. Text offsets and persisted resume anchors retain their original meaning.

Device checks: listen for recognizable pitch/speed, channel fidelity and crackling through startup, shelf navigation and book opening/return; verify mute/volume and reading silence. Inspect right edges in single/spread modes and all themes, with French apostrophes, headings and paragraph endings.
