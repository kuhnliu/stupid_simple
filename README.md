# stupid_simple
just some test code.  
put the test_thread_module.cc in webrtc/src/video/   
Add build script in webrtc/src/BUILD.gn  
    if (rtc_include_tests) {
      deps += [
        ":rtc_unittests",
        ":slow_tests",
        ":video_engine_tests",
        ":webrtc_nonparallel_tests",
        ":webrtc_perf_tests",
        "call:fake_network_unittests",
        "common_audio:common_audio_unittests",
        "common_video:common_video_unittests",
        "examples:examples_unittests",
        "media:rtc_media_unittests",
        "modules:modules_tests",
        "modules:modules_unittests",
        "modules/audio_coding:audio_coding_tests",
        "modules/audio_processing:audio_processing_tests",
        "modules/remote_bitrate_estimator:bwe_simulations_tests",
        "modules/rtp_rtcp:test_packet_masks_metrics",
        "modules/video_capture:video_capture_internal_impl",
        "pc:peerconnection_unittests",
        "pc:rtc_pc_unittests",
        "stats:rtc_stats_unittests",
        "system_wrappers:system_wrappers_unittests",
        "test",
        "video:screenshare_loopback",
        "video:sv_loopback",
        "video:video_loopback",
        "video:t_module",
      ]  

Add  build script in webrtc/src/video/BUILD.gn  
  rtc_executable("t_module") {
    testonly = true
    sources = [
      "test_thread_module.cc",
    ]
    deps = [
      "../rtc_base:checks",
      "../rtc_base:logging",
      "../rtc_base:rtc_base_approved",
      "../system_wrappers",
      "../modules/utility",
      "../system_wrappers:field_trial",
      "//third_party/abseil-cpp/absl/memory",
    ]
    if (!build_with_chromium && is_clang) {
      # Suppress warnings from the Chromium Clang plugin (bugs.webrtc.org/163).
      suppressed_configs += [ "//build/config/clang:find_bad_constructs" ]
    }
  }  
