#!/usr/bin/env python

def main ():
    """
    The main function.
    """

    import argparse
    import sys

    parser = argparse.ArgumentParser()

    parser.add_argument(
            "SOURCE",
            help = "The source file for the unit test",
            nargs = "+"
            )

    parser.add_argument(
            "--output",
            help = "The output file")

    parser.add_argument(
            "--spammpack-lib",
            help = "The spammpack library to link against",
            choices = [
                "spammpack_serial_shared",
                "spammpack_serial_static"
                ],
            default = "spammpack_serial_static"
            )

    parser.add_argument(
            "--lapack-linker-flags",
            action = "append",
            help = "Lapack linker flags")

    parser.add_argument(
            "--lapack-libraries",
            nargs = "*",
            help = "Lapack libraries")

    options = parser.parse_args()

    if options.output:
        fd = open(options.output, "w")
    else:
        import sys
        fd = sys.stdout
        sys.stderr.write("writing to stdout\n")

    fd.write("# Unit tests\n")
    fd.write("#\n")
    fd.write("# Generated by")
    for arg in sys.argv:
        fd.write(" %s" % (arg))
    fd.write("\n")
    fd.write("#\n")

    for source in options.SOURCE:
        import os.path
        testbasename = os.path.splitext(os.path.basename(source))[0]
        testexename = "unit-test-" + testbasename
        fd.write("\n")
        fd.write("# Unit test from %s\n" % (source))
        fd.write("add_executable( %s %s )\n" % (testexename, source))
        fd.write("target_link_libraries( %s\n" % (testexename))
        fd.write("  %s\n" % (options.spammpack_lib))
        for lib in options.lapack_libraries:
            fd.write("  %s\n" % (lib))
        fd.write("  utilities )\n")
        fd.write("target_include_directories( %s PRIVATE ${CMAKE_BINARY_DIR}/src )\n" % (testexename))
        fd.write("add_test( %s %s )\n" % (testbasename, testexename))
        fd.write("\n")
        fd.write("# Unit test using valgrind from %s\n" % (source))
        fd.write("if( VALGRIND )\n")
        fd.write("  add_test( valgrind-%s ${VALGRIND}\n" % (testbasename))
        fd.write("    --error-exitcode=1\n")
        fd.write("    ${CMAKE_CURRENT_BINARY_DIR}/%s )\n" % (testexename))
        fd.write("endif()\n")

    fd.close()

if __name__ == "__main__":
    main()
