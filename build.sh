echo "Creating build directory"
mkdir -p build/GUI
mkdir -p build/UDS
echo "Entering GUI directory"
cd build/GUI
gcc -c $(pkg-config --cflags gtk4) ../../GUI_GTK.c ../../sources/*.c
echo "Compiled GUI directory"
cd ../UDS
echo "Entering UDS directory"
gcc -c ../../UDS_Services/sources/{ReadDataByIdentifier.c,DiagnosticSessionControlService.c,ECUReset.c,ReadDTCInformation.c}
echo "Compiled UDS directory"
echo "Exiting build directory"
cd ../../
echo "Linking"
gcc $(pkg-config --libs gtk4) -pthread -lbluetooth build/GUI/* build/UDS/* -o diag_gui
echo "Cleaning Up"
rm -rdf build
