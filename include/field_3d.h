/**
 * \file
 * Tricubic interpolation of 3D field tables.
 */

#ifndef FIELD_3D_H_
#define FIELD_3D_H_

#include "field.h"

#include <vector>

/**
 * Class for tricubic field interpolation, create one for every table file you want to use.
 *
 * This class loads a special file format from "Vectorfields Opera" containing a regular, cuboid table of magnetic and electric fields and
 * calculates tricubic interpolation coefficients (4x4x4 = 64 for each grid point) to allow fast evaluation of the fields at arbitrary points.
 *
 */
class TabField3: public TField{
	private:
		int xl; ///< size of the table in x direction
		int yl; ///< size of the table in y direction
		int zl; ///< size of the table in z direction
		double xdist; ///< distance between grid points in x direction
		double ydist; ///< distance between grid points in y direction
		double zdist; ///< distance between grid points in z direction
		double x_mi; ///< lower x coordinate of cuboid grid
		double y_mi; ///< lower y coordinate of cuboid grid
		double z_mi; ///< lower z coordinate of cuboid grid
		std::vector<std::vector<double> > Bxc; ///< interpolation coefficients for magnetic x component
		std::vector<std::vector<double> > Byc; ///< interpolation coefficients for magnetic y component
		std::vector<std::vector<double> > Bzc; ///< interpolation coefficients for magnetic z component
		std::vector<std::vector<double> > Vc; ///< interpolation coefficients for electric potential
		double BoundaryWidth; ///< if this is larger 0, the field will be smoothly reduced to 0 in this boundary around the tabulated field cuboid
		double lengthconv; ///< Factor to convert length units in file to PENTrack units
		double Bconv; ///< Factor to convert magnetic field units in file to PENTrack units

		/**
		 * Reads an Opera table file.
		 *
		 * File has to contain x,y and z coordinates, it may contain B_x, B_y ,B_z and V columns.
		 * Sets TabField3::xl, TabField3::yl, TabField3::zl, TabField3::xdist, TabField3::ydist, TabField3::zdist, TabField3::x_mi, TabField3::y_mi, TabField3::z_mi according to the values in the table file which are used to determine the needed indeces on interpolation.
		 *
		 * @param tabfile Path to table file
		 * @param Bscale Magnetic field is always scaled by this factor
		 * @param Escale Electric field is always scaled by this factor
		 * @param BxTab Returns magnetic field x components at grid points
		 * @param ByTab Returns magnetic field y components at grid points
		 * @param BzTab Returns magnetic field z components at grid points
		 * @param VTab Returns electric potential at grid points
		 */
		void ReadTabFile(const std::string &tabfile, std::vector<double> &BxTab,
				std::vector<double> &ByTab, std::vector<double> &BzTab, std::vector<double> &VTab);


		/**
		 * Print some information for each table column
		 *
		 * @param BxTab B_x column
		 * @param ByTab B_y column
		 * @param BzTab B_z column
		 * @param VTab V column
		 */
		void CheckTab(const std::vector<double> &BxTab, const std::vector<double> &ByTab,
				const std::vector<double> &BzTab, const std::vector<double> &VTab);


		/**
		 * Calculate spatial derivatives of a table column using 1D cubic spline interpolations.
		 *
		 * @param Tab Table column of which derivatives shall be calculated
		 * @param Tab1 Returns derivatives with respect to x
		 * @param Tab2 Returns derivatives with respect to y
		 * @param Tab3 Returns derivatives with respect to z
		 * @param Tab12 Returns second derivatives with respect to x and y
		 * @param Tab13 Returns second derivatives with respect to x and z
		 * @param Tab23 Returns second derivatives with respect to y and z
		 * @param Tab123 Returns third derivatives with respect to x, y and z
		 */
		void CalcDerivs(const std::vector<double> &Tab, std::vector<double> &Tab1, std::vector<double> &Tab2, std::vector<double> &Tab3,
						std::vector<double> &Tab12, std::vector<double> &Tab13, std::vector<double> &Tab23, std::vector<double> &Tab123) const;


		/**
		 * Calculate tricubic interpolation coefficients for a table column
		 *
		 * Calls TabField3::CalcDerivs and determines the interpolation coefficients with ::tricubic_get_coeff
		 *
		 * @param coeff Returns coefficients
		 * @param Tab Table column
		 */
		void PreInterpol(std::vector<std::vector<double> > &coeff, const std::vector<double> &Tab) const;


		/**
		 * Smoothly reduce the field at the edges of the tabulated region
		 *
		 * If coordinates are within BoundaryWidth of the edges of the tabulated field,
		 * the field and its derivatives are scaled by the SmthrStp and SmthrStpDer functions.
		 *
		 * @param x x coordinate
		 * @param y y coordinate
		 * @param z z coordinate
		 * @param F Field value at (x,y,z)
		 * @param dFdxi Field derivatives at (x,y,z)
		 */
		void FieldSmthr(const double x, const double y, const double z, double &F, double dFdxi[3]) const;

		/**
		 * Smooth function used to scale the field at the edges
		 *
		 * @param x function parameter (x = 0..1)
		 *
		 * @return Returns number between 0 and 1, smoothly rising with x
		 */
		double SmthrStp(const double x) const;

		/**
		 * Derivative of SmthStpDer
		 *
		 * @param x function parameter (x = 0..1)
		 *
		 * @return Returns derivative of SmthrStp at parameter x
		 */
		double SmthrStpDer(const double x) const;
	public:
		/**
		 * Constructor.
		 *
		 * Calls TabField3::ReadTabFile, TabField3::CheckTab and for each column TabField3::PreInterpol
		 *
		 * @param tabfile Path of table file
		 * @param Bscale Time-dependent scaling formula for magnetic field
		 * @param Escale Time-dependent scaling formula for electric field
		 * @param aBoundaryWidth Sets TabField3::BoundaryWidth
		 * @param alengthconv Factor to convert length units in file to PENTrack units (default: expect cm (cgs), convert to m)
		 * @param aBconv Factor to convert magnetic field units in file to PENTrack units (default: expect Gauss (cgs), convert to Tesla)
		 */
		TabField3(const std::string &tabfile, const std::string &Bscale, const std::string &Escale,
				const double aBoundaryWidth, const double alengthconv = 0.01, const double aBconv = 1e-4);


		/**
		 * Get magnetic field at a specific point.
		 *
		 * Searches the right interpolation coefficients by determining the indices from TabField3::x_mi, TabField3::xdist, TabField3::y_mi, TabField3::ydist, TabField3::z_mi, TabField3::zdist
		 * and evaluates the interpolation polynom tricubic.h#tricubic_eval.
		 *
		 * @param x X coordinate where the field shall be evaluated
		 * @param y Y coordinate where the field shall be evaluated
		 * @param z Z coordinate where the field shall be evaluated
		 * @param t Time
		 * @param B Returns magnetic-field components
		 * @param dBidxj Returns spatial derivatives of magnetic-field components (optional)
		 */
		void BField(const double x, const double y, const double z, const double t, double B[3], double dBidxj[3][3] = NULL) const;


		/**
		 * Get electric field at a specific point.
		 *
		 * Searches the right interpolation coefficients by determining the indices from TabField3::x_mi, TabField3::xdist, TabField3::y_mi, TabField3::ydist, TabField3::z_mi, TabField3::zdist
		 * and evaluates the interpolation polynom tricubic.h#tricubic_eval.
		 *
		 * @param x X coordinate where the field shall be evaluated
		 * @param y Y coordinate where the field shall be evaluated
		 * @param z Z coordinate where the field shall be evaluated
		 * @param t Time
		 * @param V Returns electric potential
		 * @param Ei Returns electric field (negative spatial derivatives of V)
		 * @param dEidxj Adds spatial derivatives of electric field components (optional)
		 */
		void EField(const double x, const double y, const double z, const double t,
				double &V, double Ei[3], double dEidxj[3][3] = NULL) const;
};

#endif // FIELD_3D_H_
