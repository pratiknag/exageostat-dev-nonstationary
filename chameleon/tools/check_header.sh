#
#  @file check_header.sh
#
#  @copyright 2016-2021 Bordeaux INP, CNRS (LaBRI UMR 5800), Inria,
#                       Univ. Bordeaux. All rights reserved.
#
#  @version 1.1.0
#  @author Florent Pruvost
#  @author Mathieu Faverge
#  @date 2021-04-20
#
# This script check that basic informations is present and correct in
# headers of source files.
#
#!/usr/bin/env sh
header=1

print_header()
{
    if [ $header -ne 0 ]
    then
        echo "------ $1 --------"
        header=0
    fi
}

check_header_file()
{
    filename=$1
    basename=$( basename $filename .in )

    if [ "$basename" != "CMakeLists.txt" ]
    then
        toto=$( grep " @file $basename" $filename )
        if [ $? -ne 0 ]
        then
            toto=$( grep " @file .*/$basename" $filename )
        fi
        if [ $? -ne 0 ]
        then
            toto=$( grep " @file chameleon_$basename" $filename )
        fi

        if [ $? -ne 0 ]
        then
            print_header $filename
            echo -n "@file line missing or incorrect:"; grep "@file" $filename; echo ""
        fi
    fi
}

check_header_copyright()
{
    filename=$1
    basename=$( basename $filename )

    year=$( date +%Y )
    toto=$( grep -E " @copyright [0-9]{4}-$year Bordeaux INP" $filename )

    if [ $? -ne 0 ]
    then
        print_header $filename
        echo -n "@copyright line missing or incorrect:"; grep "@copyright" $filename; echo "";
    fi
}

check_header_version()
{
    filename=$1
    basename=$( basename $filename )

    toto=$( grep -E " @version [0-9]\.[0-9]\.[0-9]" $filename )
    if [ $? -ne 0 ]
    then
        print_header $filename
        echo -n "@version line missing or incorrect:"; grep "@version" $filename; echo "";
    fi
}

check_header_author()
{
    filename=$1
    basename=$( basename $filename )

    toto=$( grep -E " @author " $filename )
    if [ $? -ne 0 ]
    then
        print_header $filename
        echo "@author line missing";
    fi
}

check_header_date()
{
    filename=$1
    basename=$( basename $filename )

    toto=$( grep -E " @date [0-9]{4}-[01][0-9]-[0-3][0-9]" $filename )
    if [ $? -ne 0 ]
    then
        print_header $filename
        echo -n "@date line missing or incorrect"; grep "@date" $filename; echo "";
    fi
}

check_header_define()
{
    filename=$1
    basename=$( basename $filename )

    case $basename in
        *.h)
            n=$( basename $basename .h | awk '{print tolower($0)}' )

            macro="_${n}_h_"
            err=0

            toto=$( grep "#ifndef .*$macro" $filename )
            ret=$?
            err=$((err + ret))

            if [ $ret -eq 0 ]
            then
                macro=$( grep "#ifndef" $filename | sed 's/#ifndef //' )
            fi
            toto=$( grep "#define $macro" $filename )
            ret=$?
            err=$((err + ret))

            toto=$( grep "#endif /\* $macro \*/" $filename )
            ret=$?
            err=$((err + ret))

            if [ $err -ne 0 ]
            then
                print_header $filename
                grep "#ifndef" $filename
                grep "#define" $filename
                grep "#endif"  $filename
            fi
            ;;
        *)
    esac

}

#
# Check that the given source file contains
#
# @file filename
# @copyright
# @version
# @date
#
check_header()
{
    header=1
    check_header_file $1
    check_header_copyright $1
    check_header_version $1
    check_header_author $1
    check_header_date $1
    check_header_define $1
}

#
# Check headers
#
files=$( git ls-files                     |
             grep -v "^\."                |
             grep -v ".*\.md"             |
             grep -v ChangeLog            |
             grep -v LICENCE              |
             grep -v ".*\.cmake"          |
             grep -v "testing/lin"        |
             grep -v doc/                 |
             grep -v CTest                |
             grep -v cblas.h              |
             grep -v "lapacke.*\.h"       |
             grep -v ".*eztrace_module$"  |
             grep -v "simucore/perfmodels/\.starpu" |
             grep -v "\.xml"              |
             grep -v "input/.*\.in"       |
             grep -v "\.org"              )
if [ $# -gt 0 ]
then
    files=$*
fi

for f in $files
do
    #echo $f
    if [ -d $f ]
    then
        continue;
    fi

    check_header $f
done
