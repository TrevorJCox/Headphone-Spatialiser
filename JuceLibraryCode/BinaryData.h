/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#ifndef BINARYDATA_H_121811999_INCLUDED
#define BINARYDATA_H_121811999_INCLUDED

namespace BinaryData
{
    extern const char*   PanAziWidth_png;
    const int            PanAziWidth_pngSize = 20050;

    extern const char*   ComfortaaRegular_ttf;
    const int            ComfortaaRegular_ttfSize = 98408;

    extern const char*   Background_png;
    const int            Background_pngSize = 61958;

    extern const char*   ir1_wav;
    const int            ir1_wavSize = 110054;

    extern const char*   koli_summer_site1_4way_mono_wav;
    const int            koli_summer_site1_4way_mono_wavSize = 1440048;

    extern const char*   perth_city_hall_balcony_ir_edit_wav;
    const int            perth_city_hall_balcony_ir_edit_wavSize = 1478668;

    extern const char*   kemar_bin;
    const int            kemar_binSize = 2246400;

    // Points to the start of a list of resource names.
    extern const char* namedResourceList[];

    // Number of elements in the namedResourceList array.
    const int namedResourceListSize = 7;

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding data and its size (or a null pointer if the name isn't found).
    const char* getNamedResource (const char* resourceNameUTF8, int& dataSizeInBytes) throw();
}

#endif
