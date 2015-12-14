/**
 * \file
 * Bicubic interpolation of axisymmetric field tables.
 */

#ifndef FIELD_2D_H_
#define FIELD_2D_H_

#include <vector>

#include "interpolation.h"

#include "field.h"

/**
 * Class for bicubic field interpolation, create one for every table file you want to use.
 *
 * This class loads a special file format from "Vectorfields Opera" containing a regular, rectangular table of magnetic and electric fields and
 * calculates bicubic interpolation coefficients (4x4 matrix for each grid point) to allow fast evaluation of the fields at arbitrary points.
 * Therefore it assumes that the fields are axisymmetric around the z axis.
 *
 */
class TabField: public TField{
	private:
		int m; ///< radial size of the table file
		int n; ///< axial size of the arrays
		double rdist; ///< distance between grid points in radial direction
		double zdist; ///< distance between grid points in axial direction
		double r_mi; ///< lower radial coordinate of rectangular grid
		double z_mi; ///< lower axial coordinate of rectangular grid
		alglib::spline2dinterpolant Brc, Bphic, Bzc, Erc, Ephic, Ezc, Vc;
		double NullFieldTime; ///< Time before magnetic field is ramped (passed by constructor)
		double RampUpTime; ///< field is ramped linearly from 0 to 100% in this time (passed by constructor)
		double FullFieldTime; ///< Time the field stays at 100% (passed by constructor)
		double RampDownTime; ///< field is ramped down linearly from 100% to 0 in this time (passed by constructor)
		double lengthconv; ///< Factor to convert length units in file to PENTrack units
		double Bconv; ///< Factor to convert magnetic field units in file to PENTrack units
		double Econv; ///< Factor to convert electric field units in file to PENTrack units


		/**
		 * Reads an Opera table file.
		 *
		 * File has to contain x and z coordinates, it may contain B_x, B_y ,B_z, E_x, E_y, E_z and V columns. If V is present, E_i are ignored.
		 * Sets TabField::m, TabField::n, TabField::rdist, TabField::zdist, TabField::r_mi, TabField::z_mi according to the values in the table file which are used to determine the needed indeces on interpolation.
		 *
		 * @param tabfile Path to table file
		 * @param Bscale Magnetic field is always scaled by this factor
		 * @param Escale Electric field is always scaled by this factor
		 * @param rind Vector containing r-components of grid
		 * @param zind Vector containing z-components of grid
		 * @param BTabs Three vectors containing magnetic field components at each grid point
		 * @param ETabs Three vectors containing electric field components at each grid point
		 * @param VTab Vector containing electric potential at each grid point
		 */
		void ReadTabFile(const char *tabfile, double Bscale, double Escale, alglib::real_1d_array &rind, alglib::real_1d_array &zind,
						alglib::real_1d_array BTabs[3], alglib::real_1d_array ETabs[3], alglib::real_1d_array &VTab);


		/**
		 * Print some information for each table column
		 *
		 * @param rind Vector containing r-components of grid
		 * @param zind Vector containing z-components of grid
		 * @param BTabs Three vectors containing magnetic field components at each grid point
		 * @param ETabs Three vectors containing electric field components at each grid point
		 * @param VTab Vector containing electric potential at each grid point
		 */
		void CheckTab(alglib::real_1d_array &rind, alglib::real_1d_array &zind,
				alglib::real_1d_array BTabs[3], alglib::real_1d_array ETabs[3], alglib::real_1d_array &VTab);


		/**
		 * Get magnetic field scale factor for a specific time.
		 *
		 * Determined by TabField::NullFieldTime, TabField::RampUpTime, TabField::FullFieldTime, TabField::RampDownTime
		 *
		 * @param t Time
		 *
		 * @return Returns magnetic field scale factor
		 */
		double BFieldScale(double t);
	public:
		/**
		 * Constructor.
		 *
		 * Calls TabField::ReadTabFile, TabField::CheckTab and for each column TabField::PreInterpol
		 *
		 * @param tabfile Path of table file
		 * @param Bscale Magnetic field is always scaled by this factor
		 * @param Escale Electric field is always scaled by this factor
		 * @param aNullFieldTime Sets TabField::NullFieldTime
		 * @param aRampUpTime Sets TabField::RampUpTime
		 * @param aFullFieldTime Sets TabField::FullFieldTime
		 * @param aRampDownTime Set TabField::RampDownTime
		 * @param alengthconv Factor to convert length units in file to PENTrack units (default: expect cm (cgs), convert to m)
		 * @param aBconv Factor to convert magnetic field units in file to PENTrack units (default: expect Gauss (cgs), convert to Tesla)
		 * @param aEconv Factor to convert electric field units in file to PENTrack units (default: expect V/cm (cgs), convert to V/m)
		 */
		TabField(const char *tabfile, double Bscale, double Escale,
				double aNullFieldTime, double aRampUpTime, double aFullFieldTime, double aRampDownTime,
				double alengthconv = 0.01, double aBconv = 1e-4, double aEconv = 100.);

		~TabField();

		/**
		 * Get magnetic field at a specific point.
		 *
		 * Evaluates the interpolation polynoms and their derivatives for each field component.
		 * These radial, axial und azimuthal components have to be rotated into cartesian coordinate system.
		 *
		 * @param x X coordinate where the field shall be evaluated
		 * @param y Y coordinate where the field shall be evaluated
		 * @param z Z coordinate where the field shall be evaluated
		 * @param t Time
		 * @param B Adds magnetic field components B[0..2][0], their derivatives B[0..2][1..3], the absolute value B[3][0] and its derivatives B[3][1..3]
		 */
		void BField(double x, double y, double z, double t, double B[4][4]);


		/**
		 * Get electric field at a specific point.
		 *
		 * Evaluates the interpolation polynoms for each field component or the potential and its derivatives.
		 * These radial, axial und azimuthal components have to be rotated into cartesian coordinate system.
		 *
		 * @param x X coordinate where the field shall be evaluated
		 * @param y Y coordinate where the field shall be evaluated
		 * @param z Z coordinate where the field shall be evaluated
		 * @param t Time
		 * @param V Returns electric potential
		 * @param Ei Return electric field (negative spatial derivatives of V)
		 * @param dEidxj Adds spatial derivatives of electric field components (optional) !!!NOT YET IMPLEMENTED!!!
		 */
		void EField(double x, double y, double z, double t, double &V, double Ei[3], double dEidxj[3][3] = NULL);
};


#endif // FIELD_2D_H_
