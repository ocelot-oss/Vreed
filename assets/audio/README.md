# Library music

“Mascot's Library Visit” is original music by the Vreed project author, who confirmed authorship and authorized its distribution with Vreed on 2026-10-16.

`library.mp3` is the user-supplied “Mascot's Library Visit.mp3”, preserved without modification. GitHub Actions converts the recording with FFmpeg to signed little-endian 16-bit stereo PCM at 48 kHz, matching the existing Vita audio port. The PCM asset is packaged at `app0:/assets/audio/library-stereo.pcm`; the original MP3 is not included in the VPK.

For a local native build, install FFmpeg or prepare `library-stereo.pcm` before CMake configuration:

```sh
ffmpeg -nostdin -v error -i assets/audio/library.mp3 -ac 2 -ar 48000 -f s16le assets/audio/library-stereo.pcm
```

The dedicated audio worker reads 2048 stereo frames per block, loops at EOF and fades over 250 ms. Menu entry restarts the track; book opening stops it. Page effects mix with the fading recording using saturated output. Existing sound enable/volume preferences apply. A missing/unreadable music asset leaves paper effects and reading available.
