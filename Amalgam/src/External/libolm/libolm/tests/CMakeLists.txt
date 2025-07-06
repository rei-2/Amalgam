enable_testing()

set(TEST_LIST
    base64
    crypto
    group_session
    list
    megolm
    message
    olm
    olm_decrypt
    olm_sha256
    olm_signature
    olm_using_malloc
    session
    pk
    sas
  )

if(NOT (${CMAKE_SYSTEM_NAME} MATCHES "Windows" AND BUILD_SHARED_LIBS))
  # test_ratchet doesn't work on Windows when building a DLL, because it tries
  # to use internal symbols, so only enable it if we're not on Windows, or if
  # we're building statically
  set(TEST_LIST ${TEST_LIST} ratchet)
endif()

foreach(test IN ITEMS ${TEST_LIST})
add_executable(test_${test} test_${test}.cpp)
target_include_directories(test_${test} PRIVATE include)
target_link_libraries(test_${test} Olm::Olm)

add_test(${test} test_${test} --reporters=console,junit --out=${test}.xml)
endforeach(test)

