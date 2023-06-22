def is_within_range(range_str, check_hex):
    # Extract the start and end values from the range string
    if "-" in range_str:
        start_hex, end_hex = range_str.split("-")
    else:
        return False

    # Strip the "0x" prefix if present
    if start_hex.startswith("0x"):
        start_hex = start_hex[2:]
    if end_hex.startswith("0x"):
        end_hex = end_hex[2:]
    if check_hex.startswith("0x"):
        check_hex = check_hex[2:]

    # Convert hexadecimal numbers to integers
    start_int = int(start_hex, 16)
    end_int = int(end_hex, 16)
    check_int = int(check_hex, 16)

    # Check if the check_hex value is within the range
    return start_int <= check_int <= end_int


if __name__ == "__main__":
    range_str = input(
        "Enter a hexadecimal range (e.g. 7ffd76d19000-7ffd76d3a000): "
    )
    check_hex = input(
        "Enter a hexadecimal number to check (e.g. 7ffd76d35500): "
    )
    if is_within_range(range_str, check_hex):
        print(f"{check_hex} is within the range {range_str}")
    else:
        print(f"{check_hex} is not within the range {range_str}")
