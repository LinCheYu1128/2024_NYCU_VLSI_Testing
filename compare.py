import argparse

def remove_whitespace(text):
    return ''.join(text.split())  # Remove all spaces, newlines, tabs, etc.

def compare_files(file1, file2):
    with open(file1, 'r') as f1, open(file2, 'r') as f2:
        content1 = f1.read()
        content2 = f2.read()

    # Remove whitespace from both file contents
    content1_cleaned = remove_whitespace(content1)
    content2_cleaned = remove_whitespace(content2)

    # Compare the cleaned contents
    if content1_cleaned == content2_cleaned:
        print("The contents of the files are the same.")
    else:
        print("The contents of the files are different.")

if __name__ == "__main__":
    # Set up argument parser
    parser = argparse.ArgumentParser(description="Compare two files ignoring spaces.")
    parser.add_argument("file1", type=str, help="Path to the first file")
    parser.add_argument("file2", type=str, help="Path to the second file")

    # Parse the arguments
    args = parser.parse_args()

    # Compare the files using the provided file paths
    compare_files(args.file1, args.file2)