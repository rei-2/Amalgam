set print pretty off
set print repeats unlimited
set print elements unlimited
set breakpoint pending on


break crypto.cpp:273
commands
silent
printf "- hmac_sha256:\n"
printf "    key_length: %d\n", key_length
printf "    input_length: %d\n", input_length
if key_length > 0
    printf "    key: "
    output/x *key@key_length
    printf "\n"
else
    printf "    key: {}\n"
end
if input_length > 0
    printf "    input: "
    output/x *input@input_length
    printf "\n"
else
    printf "    input: {}\n"
end
cont
end


break crypto.cpp:280
commands
silent
printf "    output: "
output/x *output@32
printf "\n"
cont
end


break crypto.cpp:307
commands
silent
set $hkdf_output = output
cont
end


break crypto.cpp:323
commands
silent
printf "- hkdf_sha256:\n"
printf "    input_length: %d\n", input_length
printf "    salt_length: %d\n", salt_length
printf "    info_length: %d\n", info_length
printf "    output_length: %d\n", output_length
if input_length > 0
    printf "    input: "
    output/x *input@input_length
    printf "\n"
else
    printf "    input: {}\n"
end
if salt_length > 0
    printf "    salt: "
    output/x *salt@salt_length
    printf "\n"
else
    printf "    salt: {}\n"
end
if info_length > 0
    printf "    info: "
    output/x *info@info_length
    printf "\n"
else
    printf "    info: {}\n"
end
printf "    output: "
output/x *$hkdf_output@output_length
printf "\n"
cont
end


break crypto.cpp:156
commands
silent
printf "- curve25519:\n"
printf "    public: "
output/x *their_key.public_key@32
printf "\n"
printf "    private: "
output/x *our_key.private_key@32
printf "\n"
printf "    output: "
output/x *output@32
printf "\n"
cont
end

break crypto.cpp:156
commands
silent
printf "- curve25519:\n"
printf "    public: "
output/x *their_key.public_key@32
printf "\n"
printf "    private: "
output/x *our_key.private_key@32
printf "\n"
printf "    output: "
output/x *output@32
printf "\n"
cont
end

break crypto.cpp:147
commands
silent
printf "- curve25519:\n"
printf "    public: "
output/x *CURVE25519_BASEPOINT@32
printf "\n"
printf "    private: "
output/x *key_pair.private_key@32
printf "\n"
printf "    output: "
output/x *key_pair.public_key@32
printf "\n"
cont
end

run
