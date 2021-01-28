cet_enable_asserts()

# Make sure tests have correct environment settings.
include(CetTest)

# If you explicitly include CetTest in a subdirectory, you will need to
# re-initialize the test environment.
cet_test_env("FHICL_FILE_PATH=.")

cet_test(gallery_class_traits_t SOURCE class_traits_t.cc
  LIBRARIES PRIVATE canvas_root_io::canvas_root_io ROOT::Core)
