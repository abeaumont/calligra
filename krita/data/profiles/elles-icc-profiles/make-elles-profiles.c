/* License:
 * 
 * This is free software code; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for details: http://www.gnu.org/licenses/.
 * 
 * Copyright 2014 Elle Stone, 
 * ellestone@ninedegreesbelow.com
 * http://ninedegreesbelow.com
 * */

/* The profile header "Platform" tag:
 * When creating a profile, LCMS checks to see if the platform is Windows ('MSFT').
 * If your platform isn't Windows, 
 * LCMS defaults to using the Apple ('APPL') platform tag for the profile header.
 * 
 * There is an unofficial Platform cmsPlatformSignature cmsSigUnices 0x2A6E6978 '*nix'. 
 * There is, however, no LCMS2 API for changing the platform when making a profile.
 * 
 * So on my own computer, to replace 'APPL' with '*nix' in the header, 
 * I modified the LCMS source file 'cmsio0.c' and recompiled LCMS:
#ifdef CMS_IS_WINDOWS_
Header.platform= (cmsPlatformSignature) _cmsAdjustEndianess32(cmsSigMicrosoft);
#else
Header.platform= (cmsPlatformSignature) _cmsAdjustEndianess32(cmsSigUnices);
#endif
 * */

/* Command line to compile this code:
 * 
 * gcc -g -O0 -Wall -o make-profiles make-profiles.c -llcms2
 *
 * If you installed LCMS2 in /usr/local, 
 * you might need use the following command:
 * gcc -g -O0 -Wall -I/usr/local/include -o make-profiles make-profiles.c -llcms2
 * 
 * */
//cmsBool	cmsMD5computeID(cmsHPROFILE	hProfile);	
#include <lcms2.h>
int main ()
{
/* prints D50X and D50Y values from lcms2.h */
printf("D50X, D50Y, D50Z = %1.8f %1.8f %1.8f\n", cmsD50X, cmsD50Y, cmsD50Z);

/* ************************** TONE CURVES *************************** */

/* sRGB parametric tone response curve */
/* About the sRGB parametric curve: 
 * For V4 profiles, this curve works in unbounded mode.
 * For V2 profiles, the resulting TRC is a 4096-point curve and 
 * doesn't work in unbounded mode. 
 * See http://ninedegreesbelow.com/photography/lcms2-unbounded-mode.html
 * for an explanation of unbounded mode ICC profile conversions. 
 * */
cmsFloat64Number srgb_parameters[5] = 
   { 2.4, 1.0 / 1.055,  0.055 / 1.055, 1.0 / 12.92, 0.04045 }; 
cmsToneCurve *srgb_parametic_curve =
   cmsBuildParametricToneCurve(NULL, 4, srgb_parameters);
cmsToneCurve *srgb_parametric[3] = 
   {srgb_parametic_curve,srgb_parametic_curve,srgb_parametic_curve};

/* Rec 709 TRC */
cmsFloat64Number rec709_parameters[5] = 
   { 1.0 / 0.45, 1.099,  0.099, 4.500, 0.018 }; 
cmsToneCurve *rec709_parametic_curve =
   cmsBuildParametricToneCurve(NULL, 4, rec709_parameters);
cmsToneCurve *rec709_parametric[3] = 
   {rec709_parametic_curve,rec709_parametic_curve,rec709_parametic_curve};

/* l-star TRC */
cmsFloat64Number lstar_parameters[5] = 
   { 3.0, 0.862076,  0.137924, 0.110703, 0.080002 }; 
cmsToneCurve *lstar_parametic_curve =
   cmsBuildParametricToneCurve(NULL, 4, lstar_parameters);
cmsToneCurve *lstar_parametric[3] = 
   {lstar_parametic_curve,lstar_parametic_curve,lstar_parametic_curve};
   
/* gamma=1.00, linear gamma, "linear light", etc tone response curve */
cmsToneCurve* gamma100[3];
gamma100[0] = gamma100[1] = gamma100[2] = cmsBuildGamma (NULL, 1.00); 

/* Because of hexadecimal rounding during the profile making process, 
 * the following true gamma values are not precisely preserved when a V2
 * profile is made.
 * */

/* gamma=1.80078125 tone response curve */
/* http://www.color.org/chardata/rgb/ROMMRGB.pdf indicates that 
 * the official tone response curve for romm isn't a simple gamma curve 
 * but rather has a linear portion in shadows, just like sRGB.
 * Most ProPhotoRGB profiles use a simple gamma curve 
 * equal to 1.80078125.
 * This odd value is because of hexadecimal rounding.
 * When using gamma=1.80, V2 profiles end up with gamma=1.79688.
 * For this code I used gamma=1.80078125 instead of gamma=1.80.
 * Otherwise V2 and V4 versions of nominally gamma=1.80 profiles 
 * would have different gamma curves.
 * */ 
cmsToneCurve* gamma180[3];
gamma180[0] = gamma180[1] = gamma180[2] = cmsBuildGamma (NULL, 1.80078125); 


cmsToneCurve* gamma200[3];
gamma200[0] = gamma200[1] = gamma200[2] = cmsBuildGamma (NULL, 2.00);

/* gamma=2.19921875 tone response curve */
/* per http://www.adobe.com/digitalimag/pdfs/AdobeRGB1998.pdf;
 * ClayRGB uses this value. Based on an old proprietary profile, 
 * it also appears to be the correct gamma for WideGamutRGB.
 * It also is what you get when you 
 * try to create a V2 profile with a gamma=2.20 gamma curve. 
 * So perhaps the AdobeRGB1998 specifications 
 * were simply bowing to the inevitable hexadecimal rounding. 
 * Because of hexadecimal rounding errors in V2 profiles, 
 * using gamma = exactly 2.20 will result in V2 and V4 versions 
 * of the same profile having different gamma curves. 
 * Almost all (all?) V2 ICC profiles with nominally gamma=2.20 
 * really have a gamma of 2.19921875, not 2.20.
 * */
cmsToneCurve* gamma220[3];
gamma220[0] = gamma220[1] = gamma220[2] = cmsBuildGamma (NULL, 2.19921875);



/* ************************** WHITE POINTS ************************** */

/* D50 WHITE POINTS */
cmsCIExyY d50_romm_spec= {0.3457, 0.3585, 1.0};
/* http://photo-lovers.org/pdf/color/romm.pdf */
cmsCIExyY d50_illuminant_specs = {0.345702915, 0.358538597, 1.0};
/* calculated from D50 illuminant XYZ values in ICC specs */

/* D65 WHITE POINTS */
cmsCIExyY  d65_srgb_adobe_specs = {0.3127, 0.3290, 1.0};
/* White point from the sRGB.icm and AdobeRGB1998 profile specs:
 * http://www.adobe.com/digitalimag/pdfs/AdobeRGB1998.pdf 
 * 4.2.1 Reference Display White Point
 * The chromaticity coordinates of white displayed on 
 * the reference color monitor shall be x=0.3127, y=0.3290. 
 * . . . [which] correspond to CIE Standard Illuminant D65.
 * 
 * Wikipedia gives this same white point for SMPTE-C.
 * It's probably correct for most or all of the standard D65 profiles.
 * 
 * */
 
/* C and E WHITE POINTS */
cmsCIExyY c_astm  = {0.310060511, 0.316149551, 1.0}; 
/* http://www.brucelindbloom.com/index.html?Eqn_ChromAdapt.html */
cmsCIExyY e_astm  = {0.333333333, 0.333333333, 1.0};
/* see http://www.brucelindbloom.com/index.html?Eqn_ChromAdapt.html */

cmsCIExyY c_cie= {0.310, 0.316};
/* https://en.wikipedia.org/wiki/NTSC#Colorimetry */
cmsCIExyY e_cie= {0.333, 0.333};

cmsCIExyY c_6774_robertson= {0.308548930, 0.324928102, 1.0}; 
/* see http://en.wikipedia.org/wiki/Standard_illuminant#White_points_of_standard_illuminants
 * also see  http://www.brucelindbloom.com/index.html?Eqn_T_to_xy.html for the equations */
cmsCIExyY e_5454_robertson= {0.333608970, 0.348572909, 1.0}; 
/* see http://en.wikipedia.org/wiki/Standard_illuminant#White_points_of_standard_illuminants
 * also see http://www.brucelindbloom.com/index.html?Eqn_T_to_xy.html for the equations */
 
 
/* *****************Set up profile variables and values *************** */
cmsHPROFILE profile;
cmsToneCurve* tone_curve[3];
cmsCIExyY whitepoint;
cmsCIExyYTRIPLE primaries;
const char* filename;
cmsMLU *copyright = cmsMLUalloc(NULL, 1);
cmsMLUsetASCII(copyright, "en", "US", "Elle Stone, CC0, no warranty, use at your own risk; see http://creativecommons.org/publicdomain/zero/1.0/ for details.");
cmsMLU *manufacturer = cmsMLUalloc(NULL, 1);
cmsMLUsetASCII(manufacturer, "en", "US", "Elle Stone (website: http://ninedegreesbelow.com/; email: ellestone@ninedegreesbelow.com.)");
cmsMLU *description;

/* ********************** MAKE THE PROFILES ************************* */

/* D50 PROFILES */

/* ***** Make profile: AllColorsRGB, D50, gamma=1.00 */
/* AllColors.icc has a slightly larger color gamut than the ACES color space.
 * It has a D50 white point and a linear gamma TRC. It holds all visible colors.
 * Just like the ACES color space, AllColors also holds a high percentage of imaginary colors.
 * See http://ninedegreesbelow.com/photography/xyz-rgb.html#xyY 
 * for more information about imaginary colors.
 * AllColors primaries for red and blue from http://www.ledtuning.nl/cie.php
 * blue 375nm red 780nm, plus Y intercepts:
 * Color Wavelength (): 375 nm.
 * Spectral Locus coordinates: X:0.17451 Y:0.005182
 * Color Wavelength (): 780 nm.
 * Spectral Locus coordinates: X:0.734690265 Y:0.265309735
 * X1:0.17451 Y1:0.005182
 * X2:0.734690265 Y2:0.265309735
 * X3:0.00Y3:? Solve for Y3:
 * (0.265309735-0.005182)/(0.734690265-0.17451)=0.46436433279205221554=m
 * y=mx+b let x=0; y=b
 * Y1=0.005182=(0.46436433279205221554*0.17451)+b
 * b=0.005182-(0.46436433279205221554*0.17451)=-.07585421971554103213
 *  */
cmsCIExyYTRIPLE allcolors_primaries = 
{
{0.734690265,  0.265309735,  1.0},
{0.000000000,  1.000000000,  1.0},
{0.000000000, -.0758542197,  1.0}
}; 
whitepoint = d50_illuminant_specs;
primaries = allcolors_primaries;

/* linear gamma */
tone_curve[0] = tone_curve[1] = tone_curve[2] = gamma100[0];
profile = cmsCreateRGBProfile ( &whitepoint, &primaries, tone_curve );
cmsWriteTag(profile, cmsSigCopyrightTag, copyright);
/* V4 */
description = cmsMLUalloc(NULL, 1);
cmsMLUsetASCII(description, "en", "US", "AllColorsRGB-elle-V4-g10.icc");
cmsWriteTag(profile, cmsSigProfileDescriptionTag, description);
                             filename = "AllColorsRGB-elle-V4-g10.icc";
cmsSaveProfileToFile(profile, filename);
cmsMLUfree(description);
/* V2 */
cmsSetProfileVersion(profile, 2.1);
description = cmsMLUalloc(NULL, 1);
cmsMLUsetASCII(description, "en", "US", "AllColorsRGB-elle-V2-g10.icc");
cmsWriteTag(profile, cmsSigProfileDescriptionTag, description);
                             filename = "AllColorsRGB-elle-V2-g10.icc";
cmsSaveProfileToFile(profile, filename);
cmsMLUfree(description);

/* sRGB TRC */
tone_curve[0] = tone_curve[1] = tone_curve[2] = srgb_parametric[0];
profile = cmsCreateRGBProfile ( &whitepoint, &primaries, tone_curve );
cmsWriteTag(profile, cmsSigCopyrightTag, copyright);
/* V4 */
description = cmsMLUalloc(NULL, 1);
cmsMLUsetASCII(description, "en", "US", "AllColorsRGB-elle-V4-srgbtrc.icc");
cmsWriteTag(profile, cmsSigProfileDescriptionTag, description);
                             filename = "AllColorsRGB-elle-V4-srgbtrc.icc";
cmsSaveProfileToFile(profile, filename);
cmsMLUfree(description);
/* V2 */
cmsSetProfileVersion(profile, 2.1);
description = cmsMLUalloc(NULL, 1);
cmsMLUsetASCII(description, "en", "US", "AllColorsRGB-elle-V2-srgbtrc.icc");
cmsWriteTag(profile, cmsSigProfileDescriptionTag, description);
                             filename = "AllColorsRGB-elle-V2-srgbtrc.icc";
cmsSaveProfileToFile(profile, filename);
cmsMLUfree(description);


/* ***** Make profile: Identity, D50, gamma=1.00. */
/* These primaries also hold all possible visible colors, 
 * but less efficiently than the AllColors profile.*/
cmsCIExyYTRIPLE identity_primaries = {/*  */
{1.0, 0.0, 1.0},
{0.0, 1.0, 1.0},
{0.0, 0.0, 1.0}
};
whitepoint = d50_illuminant_specs;
primaries = identity_primaries;

tone_curve[0] = tone_curve[1] = tone_curve[2] = gamma100[0];
profile = cmsCreateRGBProfile ( &whitepoint, &primaries, tone_curve );
cmsWriteTag(profile, cmsSigCopyrightTag, copyright);
/* V4 */
description = cmsMLUalloc(NULL, 1);
cmsMLUsetASCII(description, "en", "US", "IdentityRGB-elle-V4-g10.icc");
cmsWriteTag(profile, cmsSigProfileDescriptionTag, description);
                             filename = "IdentityRGB-elle-V4-g10.icc";
cmsSaveProfileToFile(profile, filename);
cmsMLUfree(description);
/* V2 */
cmsSetProfileVersion(profile, 2.1);
description = cmsMLUalloc(NULL, 1);
cmsMLUsetASCII(description, "en", "US", "IdentityRGB-elle-V2-g10.icc");
cmsWriteTag(profile, cmsSigProfileDescriptionTag, description);
                             filename = "IdentityRGB-elle-V2-g10.icc";
cmsSaveProfileToFile(profile, filename);
cmsMLUfree(description);

/* sRGB TRC */
tone_curve[0] = tone_curve[1] = tone_curve[2] = srgb_parametric[0];
profile = cmsCreateRGBProfile ( &whitepoint, &primaries, tone_curve );
cmsWriteTag(profile, cmsSigCopyrightTag, copyright);
/* V4 */
description = cmsMLUalloc(NULL, 1);
cmsMLUsetASCII(description, "en", "US", "IdentityRGB-elle-V4-srgbtrc.icc");
cmsWriteTag(profile, cmsSigProfileDescriptionTag, description);
                             filename = "IdentityRGB-elle-V4-srgbtrc.icc";
cmsSaveProfileToFile(profile, filename);
cmsMLUfree(description);
/* V2 */
cmsSetProfileVersion(profile, 2.1);
description = cmsMLUalloc(NULL, 1);
cmsMLUsetASCII(description, "en", "US", "IdentityRGB-elle-V2-srgbtrc.icc");
cmsWriteTag(profile, cmsSigProfileDescriptionTag, description);
                             filename = "IdentityRGB-elle-V2-srgbtrc.icc";
cmsSaveProfileToFile(profile, filename);
cmsMLUfree(description);


/* ***** Make profile: Romm/Prophoto, D50, gamma=1.80 */
/* Reference Input/Output Medium Metric RGB Color Encodings (RIMM/ROMM RGB)
 * Kevin E. Spaulding, Geoffrey J. Woolfe and Edward J. Giorgianni
 * Eastman Kodak Company, Rochester, New York, U.S.A.
 * Above document is available at http://photo-lovers.org/pdf/color/romm.pdf
 * Kodak designed the Romm (ProPhoto) color gamut to include all printable 
 * and most real world colors. It includes some imaginary colors and excludes 
 * some of the real world blues and violet blues that can be captured by 
 * digital cameras. For high bit depth image editing only.
 */
cmsCIExyYTRIPLE romm_primaries = {
{0.7347, 0.2653, 1.0},
{0.1596, 0.8404, 1.0},
{0.0366, 0.0001, 1.0}
};
primaries = romm_primaries;
whitepoint = d50_romm_spec;
/* gamma 1.80 */
tone_curve[0] = tone_curve[1] = tone_curve[2] = gamma180[0];
profile = cmsCreateRGBProfile ( &whitepoint, &primaries, tone_curve );
cmsWriteTag(profile, cmsSigCopyrightTag, copyright);
/* V4 */
description = cmsMLUalloc(NULL, 1);
cmsMLUsetASCII(description, "en", "US", "LargeRGB-elle-V4-g18.icc");
cmsWriteTag(profile, cmsSigProfileDescriptionTag, description);
                             filename = "LargeRGB-elle-V4-g18.icc";
cmsSaveProfileToFile(profile, filename);
cmsMLUfree(description);
/* V2 */
cmsSetProfileVersion(profile, 2.1);
description = cmsMLUalloc(NULL, 1);
cmsMLUsetASCII(description, "en", "US", "LargeRGB-elle-V2-g18.icc");
cmsWriteTag(profile, cmsSigProfileDescriptionTag, description);
                             filename = "LargeRGB-elle-V2-g18.icc";
cmsSaveProfileToFile(profile, filename);
cmsMLUfree(description);

/* linear gamma */
tone_curve[0] = tone_curve[1] = tone_curve[2] = gamma100[0];
profile = cmsCreateRGBProfile ( &whitepoint, &primaries, tone_curve );
cmsWriteTag(profile, cmsSigCopyrightTag, copyright);
/* V4 */
description = cmsMLUalloc(NULL, 1);
cmsMLUsetASCII(description, "en", "US", "LargeRGB-elle-V4-g10.icc");
cmsWriteTag(profile, cmsSigProfileDescriptionTag, description);
                             filename = "LargeRGB-elle-V4-g10.icc";
cmsSaveProfileToFile(profile, filename);
cmsMLUfree(description);
/* V2 */
cmsSetProfileVersion(profile, 2.1);
description = cmsMLUalloc(NULL, 1);
cmsMLUsetASCII(description, "en", "US", "LargeRGB-elle-V2-g10.icc");
cmsWriteTag(profile, cmsSigProfileDescriptionTag, description);
                             filename = "LargeRGB-elle-V2-g10.icc";
cmsSaveProfileToFile(profile, filename);
cmsMLUfree(description);

/* sRGB TRC */
tone_curve[0] = tone_curve[1] = tone_curve[2] = srgb_parametric[0];
profile = cmsCreateRGBProfile ( &whitepoint, &primaries, tone_curve );
cmsWriteTag(profile, cmsSigCopyrightTag, copyright);
/* V4 */
description = cmsMLUalloc(NULL, 1);
cmsMLUsetASCII(description, "en", "US", "LargeRGB-elle-V4-srgbtrc.icc");
cmsWriteTag(profile, cmsSigProfileDescriptionTag, description);
                             filename = "LargeRGB-elle-V4-srgbtrc.icc";
cmsSaveProfileToFile(profile, filename);
cmsMLUfree(description);
/* V2 */
cmsSetProfileVersion(profile, 2.1);
description = cmsMLUalloc(NULL, 1);
cmsMLUsetASCII(description, "en", "US", "LargeRGB-elle-V2-srgbtrc.icc");
cmsWriteTag(profile, cmsSigProfileDescriptionTag, description);
                             filename = "LargeRGB-elle-V2-srgbtrc.icc";
cmsSaveProfileToFile(profile, filename);
cmsMLUfree(description);



/* ***** Make profile: WidegamutRGB, D50, gamma=2.19921875 */
/* Pascale's primary values produce a profile that matches 
 * old V2 Widegamut profiles from Adobe and Canon.
 * Danny Pascale: A review of RGB color spaces
 * http://www.babelcolor.com/download/A%20review%20of%20RGB%20color%20spaces.pdf  
 * WideGamutRGB was designed by Adobe to be a wide gamut color space that uses
 * spectral colors as its primaries. For high bit depth image editing only. */
cmsCIExyYTRIPLE widegamut_pascale_primaries = {
{0.7347, 0.2653, 1.0},
{0.1152, 0.8264, 1.0},
{0.1566, 0.0177, 1.0}
};
primaries = widegamut_pascale_primaries;
whitepoint = d50_romm_spec;
/* gamma 2.20 */
tone_curve[0] = tone_curve[1] = tone_curve[2] = gamma220[0];
profile = cmsCreateRGBProfile ( &whitepoint, &primaries, tone_curve );
cmsWriteTag(profile, cmsSigCopyrightTag, copyright);
/* V4 */
description = cmsMLUalloc(NULL, 1);
cmsMLUsetASCII(description, "en", "US", "WideRGB-elle-V4-g22.icc");
cmsWriteTag(profile, cmsSigProfileDescriptionTag, description);
                             filename = "WideRGB-elle-V4-g22.icc";
cmsSaveProfileToFile(profile, filename);
cmsMLUfree(description);
/* V2 */
cmsSetProfileVersion(profile, 2.1);
description = cmsMLUalloc(NULL, 1);
cmsMLUsetASCII(description, "en", "US", "WideRGB-elle-V2-g22.icc");
cmsWriteTag(profile, cmsSigProfileDescriptionTag, description);
                             filename = "WideRGB-elle-V2-g22.icc";
cmsSaveProfileToFile(profile, filename);
cmsMLUfree(description);

/* linear gamma */
tone_curve[0] = tone_curve[1] = tone_curve[2] = gamma100[0];
profile = cmsCreateRGBProfile ( &whitepoint, &primaries, tone_curve );
cmsWriteTag(profile, cmsSigCopyrightTag, copyright);
/* V4 */
description = cmsMLUalloc(NULL, 1);
cmsMLUsetASCII(description, "en", "US", "WideRGB-elle-V4-g10.icc");
cmsWriteTag(profile, cmsSigProfileDescriptionTag, description);
                             filename = "WideRGB-elle-V4-g10.icc";
cmsSaveProfileToFile(profile, filename);
cmsMLUfree(description);
/* V2 */
cmsSetProfileVersion(profile, 2.1);
description = cmsMLUalloc(NULL, 1);
cmsMLUsetASCII(description, "en", "US", "WideRGB-elle-V2-g10.icc");
cmsWriteTag(profile, cmsSigProfileDescriptionTag, description);
                             filename = "WideRGB-elle-V2-g10.icc";
cmsSaveProfileToFile(profile, filename);
cmsMLUfree(description);

/* sRGB TRC */
tone_curve[0] = tone_curve[1] = tone_curve[2] = srgb_parametric[0];
profile = cmsCreateRGBProfile ( &whitepoint, &primaries, tone_curve );
cmsWriteTag(profile, cmsSigCopyrightTag, copyright);
/* V4 */
description = cmsMLUalloc(NULL, 1);
cmsMLUsetASCII(description, "en", "US", "WideRGB-elle-V4-srgbtrc.icc");
cmsWriteTag(profile, cmsSigProfileDescriptionTag, description);
                             filename = "WideRGB-elle-V4-srgbtrc.icc";
cmsSaveProfileToFile(profile, filename);
cmsMLUfree(description);
/* V2 */
cmsSetProfileVersion(profile, 2.1);
description = cmsMLUalloc(NULL, 1);
cmsMLUsetASCII(description, "en", "US", "WideRGB-elle-V2-srgbtrc.icc");
cmsWriteTag(profile, cmsSigProfileDescriptionTag, description);
                             filename = "WideRGB-elle-V2-srgbtrc.icc";
cmsSaveProfileToFile(profile, filename);
cmsMLUfree(description);


/* D65 PROFILES */

/* ***** Make profile: ClayRGB (AdobeRGB), D65, gamma=2.19921875 */
/* The Adobe RGB 1998 color gamut covers a higher percentage of 
 * real-world greens than sRGB, but still doesn't include all printable 
 * greens, yellows, and cyans. 
 * When made using the gamma=2.19921875 tone response curve, 
 * this profile can be used for 8-bit image editing 
 * if used with appropriate caution to avoid posterization. 
 * When made with the gamma=2.19921875 tone response curve
 * this profile can be applied to DCF R98 camera-generated jpegs.
 * */
cmsCIExyYTRIPLE adobe_primaries = {
{0.6400, 0.3300, 1.0},
{0.2100, 0.7100, 1.0},
{0.1500, 0.0600, 1.0}
};
cmsCIExyYTRIPLE adobe_primaries_prequantized = {
{0.639996511, 0.329996864, 1.0},
{0.210005295, 0.710004866, 1.0},
{0.149997606, 0.060003644, 1.0}
};
primaries = adobe_primaries_prequantized;
whitepoint = d65_srgb_adobe_specs;
/* gamma 2.20 */
tone_curve[0] = tone_curve[1] = tone_curve[2] = gamma220[0];
profile = cmsCreateRGBProfile ( &whitepoint, &primaries, tone_curve );
cmsWriteTag(profile, cmsSigCopyrightTag, copyright);
/* V4 */
description = cmsMLUalloc(NULL, 1);
cmsMLUsetASCII(description, "en", "US", "ClayRGB-elle-V4-g22.icc AdobeRGB1998 primaries.");
cmsWriteTag(profile, cmsSigProfileDescriptionTag, description);
                             filename = "ClayRGB-elle-V4-g22.icc";
cmsSaveProfileToFile(profile, filename); 
cmsMLUfree(description);
/* V2 */
cmsSetProfileVersion(profile, 2.1);
description = cmsMLUalloc(NULL, 1);
cmsMLUsetASCII(description, "en", "US", "ClayRGB-elle-V2-g22.icc AdobeRGB1998 primaries.");
cmsWriteTag(profile, cmsSigProfileDescriptionTag, description);
                             filename = "ClayRGB-elle-V2-g22.icc";
cmsSaveProfileToFile(profile, filename); 
cmsMLUfree(description);

/* linear gamma */
tone_curve[0] = tone_curve[1] = tone_curve[2] = gamma100[0];
profile = cmsCreateRGBProfile ( &whitepoint, &primaries, tone_curve );
cmsWriteTag(profile, cmsSigCopyrightTag, copyright);
/* V4 */
description = cmsMLUalloc(NULL, 1);
cmsMLUsetASCII(description, "en", "US", "ClayRGB-elle-V4-g10.icc AdobeRGB1998 primaries.");
cmsWriteTag(profile, cmsSigProfileDescriptionTag, description);
                             filename = "ClayRGB-elle-V4-g10.icc";
cmsSaveProfileToFile(profile, filename); 
cmsMLUfree(description);
/* V2 */
cmsSetProfileVersion(profile, 2.1);
description = cmsMLUalloc(NULL, 1);
cmsMLUsetASCII(description, "en", "US", "ClayRGB-elle-V2-g10.icc AdobeRGB1998 primaries.");
cmsWriteTag(profile, cmsSigProfileDescriptionTag, description);
                             filename = "ClayRGB-elle-V2-g10.icc";
cmsSaveProfileToFile(profile, filename); 
cmsMLUfree(description);

/* sRGB TRC */
tone_curve[0] = tone_curve[1] = tone_curve[2] = srgb_parametric[0];
profile = cmsCreateRGBProfile ( &whitepoint, &primaries, tone_curve );
cmsWriteTag(profile, cmsSigCopyrightTag, copyright);
/* V4 */
description = cmsMLUalloc(NULL, 1);
cmsMLUsetASCII(description, "en", "US", "ClayRGB-elle-V4-srgbtrc.icc AdobeRGB1998 primaries.");
cmsWriteTag(profile, cmsSigProfileDescriptionTag, description);
                             filename = "ClayRGB-elle-V4-srgbtrc.icc";
cmsSaveProfileToFile(profile, filename); 
cmsMLUfree(description);
/* V2 */
cmsSetProfileVersion(profile, 2.1);
description = cmsMLUalloc(NULL, 1);
cmsMLUsetASCII(description, "en", "US", "ClayRGB-elle-V2-srgbtrc.icc AdobeRGB1998 primaries.");
cmsWriteTag(profile, cmsSigProfileDescriptionTag, description);
                             filename = "ClayRGB-elle-V2-srgbtrc.icc";
cmsSaveProfileToFile(profile, filename); 
cmsMLUfree(description);


/* Make profile: sRGB, D65, sRGB-trc
 * http://en.wikipedia.org/wiki/Srgb */
/* Hewlett-Packard and Microsoft designed sRGB to match 
 * the color gamut of consumer-grade CRTs from the 1990s
 * and to be the standard color space for the world wide web.
 * When made using the standard sRGB TRC, this sRGB profile 
 * can be applied to DCF R03 camera-generated jpegs and 
 * is an excellent color space for editing 8-bit images.
 * When made using the linear gamma TRC, the resulting profile
 * should only be used for high bit depth image editing.
 * */
cmsCIExyYTRIPLE srgb_primaries = {
{0.6400, 0.3300, 1.0},
{0.3000, 0.6000, 1.0},
{0.1500, 0.0600, 1.0}
};
cmsCIExyYTRIPLE srgb_primaries_pre_quantized = {
{0.639998686, 0.330010138, 1.0},
{0.300003784, 0.600003357, 1.0},
{0.150002046, 0.059997204, 1.0}
};
primaries = srgb_primaries_pre_quantized;
whitepoint = d65_srgb_adobe_specs;
/* sRGB TRC */
tone_curve[0] = tone_curve[1] = tone_curve[2] = srgb_parametric[0];
profile = cmsCreateRGBProfile ( &whitepoint, &primaries, tone_curve );
cmsWriteTag(profile, cmsSigCopyrightTag, copyright);
/* V4 */
description = cmsMLUalloc(NULL, 1);
cmsMLUsetASCII(description, "en", "US", "sRGB-elle-V4-srgbtrc.icc");
cmsWriteTag(profile, cmsSigProfileDescriptionTag, description);
                             filename = "sRGB-elle-V4-srgbtrc.icc";
cmsSaveProfileToFile(profile, filename);
cmsMLUfree(description);
/* V2 */
cmsSetProfileVersion(profile, 2.1);
description = cmsMLUalloc(NULL, 1);
cmsMLUsetASCII(description, "en", "US", "sRGB-elle-V2-srgbtrc.icc");
cmsWriteTag(profile, cmsSigProfileDescriptionTag, description);
                             filename = "sRGB-elle-V2-srgbtrc.icc";
cmsSaveProfileToFile(profile, filename);
cmsMLUfree(description);

/* linear gamma */
tone_curve[0] = tone_curve[1] = tone_curve[2] = gamma100[0];
profile = cmsCreateRGBProfile ( &whitepoint, &primaries, tone_curve );
cmsWriteTag(profile, cmsSigCopyrightTag, copyright);
/* V4 */
description = cmsMLUalloc(NULL, 1);
cmsMLUsetASCII(description, "en", "US", "sRGB-elle-V4-g10.icc");
cmsWriteTag(profile, cmsSigProfileDescriptionTag, description);
                             filename = "sRGB-elle-V4-g10.icc";
cmsSaveProfileToFile(profile, filename);
cmsMLUfree(description);
/* V2 */
cmsSetProfileVersion(profile, 2.1);
description = cmsMLUalloc(NULL, 1);
cmsMLUsetASCII(description, "en", "US", "sRGB-elle-V2-g10.icc");
cmsWriteTag(profile, cmsSigProfileDescriptionTag, description);
                             filename = "sRGB-elle-V2-g10.icc";
cmsSaveProfileToFile(profile, filename);
cmsMLUfree(description);



/* The CIE-RGB profile, E white point: */
/* The ASTM E white point is probably the right white point 
 * to use when making the CIE-RGB color space profile.
 * It's not clear to me what the correct CIE-RGB primaries really are.
 * Lindbloom gives one set. The LCMS version 1 tutorial gives a different set.
 * I asked a friend to ask an expert, who said the real primaries should
 * be calculated from the spectral wavelengths. 
 * Two sets of primaries are given below:
 * */
/* This page explains what the CIE color space is:
 * https://en.wikipedia.org/wiki/CIE_1931
 * These pages give the wavelengths:
 * http://hackipedia.org/Color%20space/pdf/CIE%20Color%20Space.pdf 
 * http://infocom.ing.uniroma1.it/~gaetano/texware/Full-How%20the%20CIE%201931%20Color-Matching%20Functions%20Were%20Derived%20from%20Wright-Guild%20Data.pdf
 * This page has resources for calculating xy values given a spectral color wavelength:
 * http://www.cvrl.org/cmfs.htm
 * This page does the calculations for you:
 * http://www.ledtuning.nl/cie.php
 * Plugging the wavelengths into the ledtuning website 
 * gives the following CIE RGB xy primaries:
700.0 nm has Spectral Locus coordinates: x:0.734690023  y:0.265309977
546.1 nm has Spectral Locus coordinates: x:0.2736747378 y:0.7174284409
435.8 nm has Spectral Locus coordinates: x:0.1665361196 y:0.0088826412
* */
cmsCIExyYTRIPLE cie_primaries_ledtuning = {
{0.7346900230, 0.2653099770, 1.0},
{0.2736747378, 0.7174284409, 1.0},
{0.1665361196, 0.0088826412, 1.0}
};
/* Assuming you want to use the ASTM values for the E white point, 
 * here are the prequantized ledtuning primaries */
cmsCIExyYTRIPLE cie_primaries_ledtuning_prequantized = {
{0.734689082, 0.265296653, 1.0},
{0.273673341, 0.717437354, 1.0},
{0.166531028, 0.008882428, 1.0}
};
primaries = cie_primaries_ledtuning_prequantized;
whitepoint = e_astm;
tone_curve[0] = tone_curve[1] = tone_curve[2] = gamma220[0];
profile = cmsCreateRGBProfile ( &whitepoint, &primaries, tone_curve );
cmsWriteTag(profile, cmsSigCopyrightTag, copyright);
/* V4 */
description = cmsMLUalloc(NULL, 1);
cmsMLUsetASCII(description, "en", "US", "CIERGB-elle-V4-g22.icc");
cmsWriteTag(profile, cmsSigProfileDescriptionTag, description);
                             filename = "CIERGB-elle-V4-g22.icc";
cmsSaveProfileToFile(profile, filename);
cmsMLUfree(description);
/* V2 */
cmsSetProfileVersion(profile, 2.1);
description = cmsMLUalloc(NULL, 1);
cmsMLUsetASCII(description, "en", "US", "CIERGB-elle-V2-g22.icc");
cmsWriteTag(profile, cmsSigProfileDescriptionTag, description);
                             filename = "CIERGB-elle-V2-g22.icc";
cmsSaveProfileToFile(profile, filename);
cmsMLUfree(description);

/* A linear gamma version of this profile makes more sense 
 * than a gamma=2.2 version */
tone_curve[0] = tone_curve[1] = tone_curve[2] = gamma100[0];
whitepoint = e_astm;
profile = cmsCreateRGBProfile ( &whitepoint, &primaries, tone_curve );
cmsWriteTag(profile, cmsSigCopyrightTag, copyright);
/* V4 */
description = cmsMLUalloc(NULL, 1);
cmsMLUsetASCII(description, "en", "US", "CIERGB-elle-V4-g10.icc");
cmsWriteTag(profile, cmsSigProfileDescriptionTag, description);
                             filename = "CIERGB-elle-V4-g10.icc";
cmsSaveProfileToFile(profile, filename);
cmsMLUfree(description);
/* V2 */
cmsSetProfileVersion(profile, 2.1);
description = cmsMLUalloc(NULL, 1);
cmsMLUsetASCII(description, "en", "US", "CIERGB-elle-V2-g10.icc");
cmsWriteTag(profile, cmsSigProfileDescriptionTag, description);
                             filename = "CIERGB-elle-V2-g10.icc";
cmsSaveProfileToFile(profile, filename);
cmsMLUfree(description);

/* sRGB TRC*/
tone_curve[0] = tone_curve[1] = tone_curve[2] = srgb_parametric[0];
whitepoint = e_astm;
profile = cmsCreateRGBProfile ( &whitepoint, &primaries, tone_curve );
cmsWriteTag(profile, cmsSigCopyrightTag, copyright);
/* V4 */
description = cmsMLUalloc(NULL, 1);
cmsMLUsetASCII(description, "en", "US", "CIERGB-elle-V4-srgbtrc.icc");
cmsWriteTag(profile, cmsSigProfileDescriptionTag, description);
                             filename = "CIERGB-elle-V4-srgbtrc.icc";
cmsSaveProfileToFile(profile, filename);
cmsMLUfree(description);
/* V2 */
cmsSetProfileVersion(profile, 2.1);
description = cmsMLUalloc(NULL, 1);
cmsMLUsetASCII(description, "en", "US", "CIERGB-elle-V2-srgbtrc.icc");
cmsWriteTag(profile, cmsSigProfileDescriptionTag, description);
                             filename = "CIERGB-elle-V2-srgbtrc.icc";
cmsSaveProfileToFile(profile, filename);
cmsMLUfree(description);


/* 
 * Here's the second set of primaries
 * http://www.cis.rit.edu/research/mcsl2/research/broadbent/CIE1931_RGB.pdf
 * https://groups.google.com/forum/#!topic/sci.engr.color/fBI3k1llm-g 
 * Lindbloom gives these values on his Working Space Information page: */
cmsCIExyYTRIPLE cie_primaries_lindbloom = {
{0.7350, 0.2650, 1.0},
{0.2740, 0.7170, 1.0},
{0.1670, 0.0090, 1.0}
}; 
/* Assuming you want to use the ASTM values for the E white point, 
 * here are the prequantized Lindbloom primaries */
cmsCIExyYTRIPLE cie_primaries_lindbloom_prequantized = {
{0.714504840, 0.297234644, 1.0},
{0.520085568, 0.452364535, 1.0},
{0.090957433, 0.051485032, 1.0}
}; 


/* ***** Make profile: Gray ICC profiles - D50 white point  ********* */
whitepoint = d50_illuminant_specs;
const cmsToneCurve* grayTRC;

/* Gray profile with gamma=1.00, linear gamma, "linear light", etc */
grayTRC = cmsBuildGamma (NULL, 1.00); 
profile = cmsCreateGrayProfile ( &whitepoint, grayTRC );
cmsWriteTag(profile, cmsSigCopyrightTag, copyright);
/* V4 */
description = cmsMLUalloc(NULL, 1);
cmsMLUsetASCII(description, "en", "US", "Gray-D50-elle-V4-g10.icc");
cmsWriteTag(profile, cmsSigProfileDescriptionTag, description);
                             filename = "Gray-D50-elle-V4-g10.icc";
cmsSaveProfileToFile(profile, filename);
cmsMLUfree(description);
/* V2 */
cmsSetProfileVersion(profile, 2.1);
description = cmsMLUalloc(NULL, 1);
cmsMLUsetASCII(description, "en", "US", "Gray-D50-elle-V2-g10.icc");
cmsWriteTag(profile, cmsSigProfileDescriptionTag, description);
                             filename = "Gray-D50-elle-V2-g10.icc";
cmsSaveProfileToFile(profile, filename);
cmsMLUfree(description);

/* Gray profile with gamma=1.80, 
 * actually 1.80078125,
 * in order to create the same gamma curve in V2 and V4 profiles */
grayTRC = cmsBuildGamma (NULL, 1.80078125); 
profile = cmsCreateGrayProfile ( &whitepoint, grayTRC );
cmsWriteTag(profile, cmsSigCopyrightTag, copyright);
/* V4 */
description = cmsMLUalloc(NULL, 1);
cmsMLUsetASCII(description, "en", "US", "Gray-D50-elle-V4-g18.icc");
cmsWriteTag(profile, cmsSigProfileDescriptionTag, description);
                             filename = "Gray-D50-elle-V4-g18.icc";
cmsSaveProfileToFile(profile, filename);
cmsMLUfree(description);
/* V2 */
cmsSetProfileVersion(profile, 2.1);
description = cmsMLUalloc(NULL, 1);
cmsMLUsetASCII(description, "en", "US", "Gray-D50-elle-V2-g18.icc");
cmsWriteTag(profile, cmsSigProfileDescriptionTag, description);
                             filename = "Gray-D50-elle-V2-g18.icc";
cmsSaveProfileToFile(profile, filename);
cmsMLUfree(description);

/* Gray profile with gamma=2.20 
 * actually gamma=2.19921875,
 * in order to create the same gamma curve in V2 and V4 profiles  */
grayTRC = cmsBuildGamma (NULL, 2.19921875); 
profile = cmsCreateGrayProfile ( &whitepoint, grayTRC );
cmsWriteTag(profile, cmsSigCopyrightTag, copyright);
/* V4 */
description = cmsMLUalloc(NULL, 1);
cmsMLUsetASCII(description, "en", "US", "Gray-D50-elle-V4-g22.icc");
cmsWriteTag(profile, cmsSigProfileDescriptionTag, description);
                             filename = "Gray-D50-elle-V4-g22.icc";
cmsSaveProfileToFile(profile, filename);
cmsMLUfree(description);
/* V2 */
cmsSetProfileVersion(profile, 2.1);
description = cmsMLUalloc(NULL, 1);
cmsMLUsetASCII(description, "en", "US", "Gray-D50-elle-V2-g22.icc");
cmsWriteTag(profile, cmsSigProfileDescriptionTag, description);
                             filename = "Gray-D50-elle-V2-g22.icc";
cmsSaveProfileToFile(profile, filename);
cmsMLUfree(description);

/* Gray profile with srgb-trc */
grayTRC = cmsBuildParametricToneCurve(NULL, 4, srgb_parameters);
profile = cmsCreateGrayProfile ( &whitepoint, grayTRC );
cmsWriteTag(profile, cmsSigCopyrightTag, copyright);
/* V4 */
description = cmsMLUalloc(NULL, 1);
cmsMLUsetASCII(description, "en", "US", "Gray-D50-elle-V4-srgbtrc.icc");
cmsWriteTag(profile, cmsSigProfileDescriptionTag, description);
                             filename = "Gray-D50-elle-V4-srgbtrc.icc";
cmsSaveProfileToFile(profile, filename);
cmsMLUfree(description);
/* V2 */
cmsSetProfileVersion(profile, 2.1);
description = cmsMLUalloc(NULL, 1);
cmsMLUsetASCII(description, "en", "US", "Gray-D50-elle-V2-srgbtrc.icc");
cmsWriteTag(profile, cmsSigProfileDescriptionTag, description);
                             filename = "Gray-D50-elle-V2-srgbtrc.icc";
cmsSaveProfileToFile(profile, filename);
cmsMLUfree(description);


/* ***** Make profile: LCMS built-in LAB and XYZ profiles *********** */
/* Based on transicc output, the V4 profiles 
 * can be used in unbounded mode, but the V2 versions cannot. */
profile  = cmsCreateLab2Profile(&d50_illuminant_specs);
cmsWriteTag(profile, cmsSigCopyrightTag, copyright);
description = cmsMLUalloc(NULL, 1);
cmsMLUsetASCII(description, "en", "US", "Lab-D50-Identity-elle-V2.icc");
                             filename = "Lab-D50-Identity-elle-V2.icc";
cmsSaveProfileToFile(profile, filename);
cmsMLUfree(description);

profile  = cmsCreateLab4Profile(&d50_illuminant_specs);
cmsWriteTag(profile, cmsSigCopyrightTag, copyright);
description = cmsMLUalloc(NULL, 1);
cmsMLUsetASCII(description, "en", "US", "Lab-D50-Identity-elle-V4.icc");
                             filename = "Lab-D50-Identity-elle-V4.icc";
cmsSaveProfileToFile(profile, filename);
cmsMLUfree(description);

profile = cmsCreateXYZProfile();
cmsWriteTag(profile, cmsSigCopyrightTag, copyright);
description = cmsMLUalloc(NULL, 1);
cmsMLUsetASCII(description, "en", "US", "XYZ-D50-Identity-elle-V4.icc");
                             filename = "XYZ-D50-Identity-elle-V4.icc";
cmsSaveProfileToFile(profile, filename);
cmsMLUfree(description);


/* For the following profiles, information is provided, but not the actual
 * profile making code.
 * */

/* old monitor-based editing profiles */

/* ColorMatchRGB, D50, gamma=1.80 */
/* http://www.dpreview.com/forums/post/3902882 
 * http://lists.apple.com/archives/colorsync-users/2001/Apr/msg00073.html
 * ColorMatch was designed to fit Radius PressView CRT monitors,
 * similar to sRGB fitting consumer-grade CRT monitors, 
 * "fit" meaning "could be calibrated to match".
 * Adobe does still distribute a ColorMatchRGB profile.
 * Making this profile using the D50_romm_doc white point that is used 
 * in other old V2 profiles (ProPhoto, WideGamut)
 * doesn't make a well behaved profile,
 * but the resulting profile is very close to the Adobe-supplied version. 
 * Using the prequantized primaries makes a profile that's just as close
 * to the Adobe-supplied version and in addition is well behaved.
 * Unless you have untagged images created on a PressView CRT, 
 * there is no reason to make or use this profile. 
 * */
cmsCIExyYTRIPLE colormatch_primaries = {
{0.6300, 0.3400, 1.0},
{0.2950, 0.6050, 1.0},
{0.1500, 0.0750, 1.0}
};
cmsCIExyYTRIPLE colormatch_primaries_prequantized = {
{0.629992636, 0.339999723, 1.0},
{0.295006332, 0.604997745, 1.0},
{0.149992036, 0.075005244, 1.0}
};
primaries = colormatch_primaries_prequantized;
whitepoint = d50_romm_spec;
tone_curve[0] = tone_curve[1] = tone_curve[2] = gamma180[0];

/* AppleRGB, D65, gamma=1.80 */
/* AppleRGB was created to fit the old Apple CRT displays
 * just as sRGB fit consumer-grade CRT monitors
 * and ColorMatch fit PressView CRT monitors.
 * */
cmsCIExyYTRIPLE apple_primaries = {
{0.6250, 0.3400, 1.0},
{0.2800, 0.5950, 1.0},
{0.1550, 0.0700, 1.0}
};
cmsCIExyYTRIPLE apple_primaries_prequantized = {
{0.625012368, 0.340000081, 1.0},
{0.279996113, 0.595006943, 1.0},
{0.155001212, 0.070001183, 1.0}
};
primaries = apple_primaries_prequantized;
whitepoint = d65_srgb_adobe_specs;
tone_curve[0] = tone_curve[1] = tone_curve[2] = gamma180[0];

/* video profiles */

/* PAL-SECAM, D65 gamma=2.20 */
/* http://en.wikipedia.org/wiki/PAL */
cmsCIExyYTRIPLE pal_primaries = {
{0.6400, 0.3300, 1.0},
{0.2900, 0.6000, 1.0},
{0.1500, 0.0600, 1.0}
}; 
/* PAL is one of many video and television-related color spaces. 
 * If you need the original profile with all its tags, 
 * I recommend that you use the Argyllcms version of this profile 
 * (EBU3213_PAL.icm, located in the "ref" folder), 
 * rather than making your own. 
 * But if you do want to make your own PAL profile using LCMS2, 
 * these prequantized primaries and white point make a profile with 
 * the same primaries and white point as the Argyllcms profile. 
 * The Argyllcms profile has a point curve TRC.
 * */
cmsCIExyYTRIPLE pal_primaries_prequantized = {
{0.640007798, 0.330006592, 1.0},
{0.290000327, 0.600000840, 1.0},
{0.149998025, 0.059996098, 1.0}
};
primaries = pal_primaries_prequantized;
whitepoint = d65_srgb_adobe_specs;
tone_curve[0] = tone_curve[1] = tone_curve[2] = gamma220[0];

/* SMPTE-C, D65, gamma=2.20 */
/* http://en.wikipedia.org/wiki/NTSC#SMPTE_C 
 * SMPTE-C is one of many video and television-related color spaces 
 * and is an update of the original NTSC. */
cmsCIExyYTRIPLE smpte_c_primaries = {
{0.6300, 0.3400, 1.0},
{0.3100, 0.5950, 1.0},
{0.1550, 0.0700, 1.0}
};
cmsCIExyYTRIPLE smpte_c_primaries_prequantized = {
{0.629996495, 0.339990597, 1.0},
{0.309997880, 0.594995808, 1.0},
{0.149999952, 0.069999431, 1.0}
};
primaries = smpte_c_primaries_prequantized;
whitepoint = d65_srgb_adobe_specs;
tone_curve[0] = tone_curve[1] = tone_curve[2] = gamma220[0];

/* NTSC, C, gamma=2.20 */
/* http://en.wikipedia.org/wiki/NTSC#Colorimetry*/
/* According to Wikipedia, these "original 1953 color NTSC specifications"
 * were used by early television receivers. */
cmsCIExyYTRIPLE ntcs_primaries = {
{0.6700, 0.3300, 1.0},
{0.2100, 0.7100, 1.0},
{0.1400, 0.0800, 1.0}
}; 
cmsCIExyYTRIPLE ntcs_primaries_prequantized = {
{0.670010373, 0.330001186, 1.0},
{0.209999261, 0.710001124, 1.0},
{0.139996061, 0.080002934, 1.0}
};
primaries = ntcs_primaries_prequantized;
whitepoint = c_astm;
tone_curve[0] = tone_curve[1] = tone_curve[2] = gamma220[0];


/* ***********************  wrap up and close out  ****************** */

/* free copyright */
cmsMLUfree(copyright);

/* make gcc happy by returning an integer from main() */
return 0;
}
