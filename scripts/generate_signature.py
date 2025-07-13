#!/usr/bin/env python3
"""
AmalgamLoader Build-Time Signature Generator
Creates unique signatures for each build to ensure different file hashes
"""

import os
import sys
import hashlib
import random
import string
from datetime import datetime

def generate_unique_build_id():
    """Generate a unique build identifier"""
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    random_suffix = ''.join(random.choices(string.ascii_uppercase + string.digits, k=8))
    return f"{timestamp}_{random_suffix}"

def generate_dummy_resources(build_id, output_dir):
    """Generate dummy resources that vary per build"""
    dummy_data = []
    
    # Generate random padding data
    for i in range(random.randint(10, 50)):
        dummy_data.append(f"// Build signature: {build_id}")
        dummy_data.append(f"// Random data {i}: {''.join(random.choices(string.ascii_letters + string.digits, k=32))}")
    
    # Write to a header file that gets included in the build
    header_path = os.path.join(output_dir, "build_signature.h")
    with open(header_path, 'w') as f:
        f.write("#pragma once\n\n")
        f.write("// Auto-generated build signature - DO NOT EDIT\n")
        f.write(f'#define BUILD_SIGNATURE "{build_id}"\n')
        f.write(f'#define BUILD_TIMESTAMP "{datetime.now().isoformat()}"\n')
        f.write("\n")
        
        # Add dummy data as comments to change file hash
        for line in dummy_data:
            f.write(f"{line}\n")
        
        # Add dummy variables that get optimized out but affect compilation
        f.write("\nnamespace BuildSignature {\n")
        f.write(f'    constexpr const char* SIGNATURE = "{build_id}";\n')
        f.write(f'    constexpr const char* TIMESTAMP = "{datetime.now().isoformat()}";\n')
        f.write("}\n")
    
    print(f"Generated build signature: {build_id}")
    print(f"Signature header: {header_path}")
    return build_id

def modify_version_info(build_id, rc_file_path):
    """Modify version resource to include unique build info"""
    if not os.path.exists(rc_file_path):
        return
    
    # Read the RC file
    with open(rc_file_path, 'r') as f:
        content = f.read()
    
    # Add unique version info
    version_addition = f'''
// Build signature comments - vary per build
// {build_id}
// Generated: {datetime.now()}
// Random: {''.join(random.choices(string.ascii_letters + string.digits, k=64))}
'''
    
    # Insert at the end of the file
    content += version_addition
    
    with open(rc_file_path, 'w') as f:
        f.write(content)
    
    print(f"Modified RC file with unique signature")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python generate_signature.py <output_directory> [rc_file]")
        sys.exit(1)
    
    output_dir = sys.argv[1]
    rc_file = sys.argv[2] if len(sys.argv) > 2 else None
    
    os.makedirs(output_dir, exist_ok=True)
    
    build_id = generate_unique_build_id()
    generate_dummy_resources(build_id, output_dir)
    
    if rc_file:
        modify_version_info(build_id, rc_file)
    
    print(f"Build signature generation complete: {build_id}")