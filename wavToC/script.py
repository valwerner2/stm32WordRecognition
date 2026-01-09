import sys
import wave
import struct
import os
import re


def sanitize_name(filename):
    """
    Turns a filename into a valid C variable name.
    1. Removes extensions.
    2. Replaces non-alphanumeric characters with underscores.
    3. Ensures it doesn't start with a number.
    """
    # Remove file extension
    base_name = os.path.splitext(os.path.basename(filename))[0]

    # Replace invalid characters with underscores
    clean_name = re.sub(r'[^a-zA-Z0-9]', '_', base_name)

    # Ensure it doesn't start with a digit
    if clean_name[0].isdigit():
        clean_name = '_' + clean_name

    return clean_name


def read_wav_data(filepath):
    """
    Reads a WAV file and returns a list of floats normalized between -1.0 and 1.0.
    Supports 8-bit (unsigned) and 16-bit (signed) PCM WAV files.
    """
    try:
        with wave.open(filepath, 'rb') as wav_file:
            # Extract Audio Parameters
            n_channels = wav_file.getnchannels()
            sampwidth = wav_file.getsampwidth()
            n_frames = wav_file.getnframes()

            # Read raw bytes
            raw_data = wav_file.readframes(n_frames)

            # Calculate total samples (frames * channels)
            total_samples = n_frames * n_channels

            if sampwidth == 1:
                # 8-bit audio is Unsigned (0 to 255)
                # Center at 128 to make it signed (-128 to 127)
                fmt = f"{total_samples}B"  # B is unsigned char
                unpacked_data = struct.unpack(fmt, raw_data)
                # Normalize: (x - 128) / 128.0
                return [(x - 128) / 128.0 for x in unpacked_data]

            elif sampwidth == 2:
                # 16-bit audio is Signed (-32768 to 32767)
                fmt = f"{total_samples}h"  # h is short (signed 16-bit)
                unpacked_data = struct.unpack(fmt, raw_data)
                # Normalize: x / 32768.0
                return [x / 32768.0 for x in unpacked_data]

            else:
                print(f"Error: Unsupported sample width: {sampwidth * 8}-bit. Only 8-bit and 16-bit are supported.")
                sys.exit(1)

    except wave.Error as e:
        print(f"Error reading WAV file: {e}")
        sys.exit(1)
    except FileNotFoundError:
        print(f"Error: File '{filepath}' not found.")
        sys.exit(1)


def generate_c_file(input_wav, output_c):
    # Prepare data
    float_data = read_wav_data(input_wav)
    var_name = sanitize_name(input_wav)
    array_len = len(float_data)

    # Generate C Content
    c_content = []
    c_content.append(f"// Generated from {input_wav}")
    c_content.append(f"// Sample count: {array_len}")
    c_content.append(f"#ifndef {var_name.upper()}_H")
    c_content.append(f"#define {var_name.upper()}_H")
    c_content.append("")
    c_content.append(f"unsigned int {var_name}_len = {array_len};")
    c_content.append("")
    c_content.append(f"float {var_name}[] = {{")

    # Format data for readability (10 items per line)
    line_buffer = []
    for i, sample in enumerate(float_data):
        line_buffer.append(f"{sample:.6f}f")
        if len(line_buffer) >= 10:
            c_content.append("    " + ", ".join(line_buffer) + ",")
            line_buffer = []

    # Add remaining items
    if line_buffer:
        c_content.append("    " + ", ".join(line_buffer))

    c_content.append("};")
    c_content.append("")
    c_content.append("#endif")

    # Write to file
    with open(output_c, 'w') as f:
        f.write("\n".join(c_content))

    print(f"Success! Converted '{input_wav}' to '{output_c}'")
    print(f"Created float array '{var_name}' with {array_len} samples.")


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 script.py <input_wav_file> [output_file]")
        sys.exit(1)

    input_file = sys.argv[1]

    # Determine output filename
    if len(sys.argv) >= 3:
        output_file = sys.argv[2]
    else:
        # Default to filename.h
        output_file = sanitize_name(input_file) + ".h"

    generate_c_file(input_file, output_file)