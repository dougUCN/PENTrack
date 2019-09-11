#include "geometry.h"

#include <iostream>
#include <algorithm>

#include "globals.h"

using namespace std;

std::istream& operator>>(std::istream &str, material &mat){
	str >> mat.FermiReal >> mat.FermiImag >> mat.DiffProb >> mat.SpinflipProb
				>> mat.RMSRoughness >> mat.CorrelLength >> mat.LossPerBounce >> mat.MFPElastic;
	if (!str)
		throw std::runtime_error((boost::format("Could not read material %s!") % mat.name).str());
	if (mat.DiffProb != 0 && (mat.RMSRoughness != 0 || mat.CorrelLength != 0))
		throw std::runtime_error((boost::format("You have set both LambertReflectionProbability and RMSRoughness/CorrelationLength for material %s! "\
												"Now I don't know if I should use Lambertian reflection or micro-roughness reflection.") % mat.name).str());
	return str;
}
std::ostream& operator<<(std::ostream &str, const material &mat){
	str << mat.name << " " << mat.FermiReal << " " << mat.FermiImag << " " << mat.DiffProb << " " << mat.SpinflipProb << " "
	 			<< mat.RMSRoughness << " " << mat.CorrelLength << " " << mat.LossPerBounce << " " << mat.MFPElastic << "\n";
	if (!str)
		throw std::runtime_error((boost::format("Could not write material %s!") % mat.name).str());
	return str;
}

std::istream& operator>>(std::istream &str, solid &model){
	str >> model.filename >> model.mat.name;
	if (!str)
		throw std::runtime_error((boost::format("Could not load solid with ID %d! Did you define invalid parameters?") % model.ID).str());

	double ignorestart, ignoreend;
	while (str){
		str >> ignorestart;
		if (!str) // no more ignore times found
			break;
		char dash;
		str >> dash;
		str >> ignoreend;
		if (str && dash == '-'){
			model.ignoretimes.push_back(std::make_pair(ignorestart, ignoreend));
		}
		else{
			throw std::runtime_error((boost::format("Invalid ignoretimes for solid %d") % model.ID).str());
		}
	}
	return str;
}

std::ostream& operator<<(std::ostream &str, const solid &sld){
	str << sld.ID << " " << sld.filename << " " << sld.mat.name;
	for (auto i : sld.ignoretimes)
		str << " " << i.first << "-" << i.second;
	if (!str)
		throw std::runtime_error((boost::format("Could not write solid %d!") % sld.ID).str());
	return str;
}

TGeometry::TGeometry(TConfig &geometryin){
	boost::filesystem::path matpath;
	istringstream(geometryin["GLOBAL"]["materials_file"]) >> matpath; // check if there is a materials file linked in the config file
	TConfig matconf;
	if (matpath.empty()){ // if there is no materials file given load materials from config file
		matconf = geometryin;
		cout << "Loading materials from " << configpath << "\n";
	}
	else{
		matpath = boost::filesystem::absolute(matpath, configpath.parent_path()); // make path absolute, relative paths are assumed to be relative to the config file's path
		cout << "Loading materials from " << matpath << "\n";
		matconf.ReadFromFile(matpath.native());
	}

	vector<material> materials;
	std::transform(matconf["MATERIALS"].begin(), matconf["MATERIALS"].end(), back_inserter(materials),
					[](const std::pair<std::string, std::string> &i){
						material mat;
						mat.name = i.first;
						istringstream(i.second) >> mat;
						return mat;
					}
	); // Read materials from config and add them to list

	for (auto sldparams : geometryin["GEOMETRY"]){
		solid sld;
		istringstream(sldparams.first) >> sld.ID;
		istringstream(sldparams.second) >> sld;

		auto mat = std::find_if(materials.begin(), materials.end(), [&sld](const material &m){ return sld.mat.name == m.name; });
		if (mat == materials.end())
			throw std::runtime_error((boost::format("Material %s used but not defined!") % sld.mat.name).str());
		sld.mat = *mat;

		if (sld.ID == 1){
			sld.name = "default solid";
			defaultsolid = sld;
		}
		else{
			sld.name = mesh.ReadFile(boost::filesystem::absolute(sld.filename, configpath.parent_path()).native(), sld.ID);
			solids.push_back(sld);
		}
	}

	if (std::unique(solids.begin(), solids.end(), [](const solid s1, const solid s2){ return s1.ID == s2.ID; }) != solids.end()) // check if IDs of each solid are unique
		throw std::runtime_error("You defined solids with identical ID! IDs have to be unique!");
}


bool TGeometry::GetCollisions(const double x1, const double p1[3], const double x2, const double p2[3], multimap<TCollision, bool> &colls) const{
	vector<TCollision> c = mesh.Collision(std::vector<double>{p1[0], p1[1], p1[2]}, std::vector<double>{p2[0], p2[1], p2[2]});
	colls.clear();
	for (auto it: c){
		solid sld = GetSolid(it.ID);
		double t = x1 + (x2 - x1)*it.s;
		bool ignored =std::any_of(sld.ignoretimes.begin(), sld.ignoretimes.end(),
						[&t](const std::pair<double, double> &its){ return t >= its.first && t < its.second; }
					); // check if collision time lies between any pair of ignore times
		colls.emplace(it, ignored);
	}
	return !colls.empty();
}


std::vector<std::pair<solid, bool> > TGeometry::GetSolids(const double t, const double p[3]) const{
	double p2[3] = {mesh.GetBoundingBox().xmin() - REFLECT_TOLERANCE,
			mesh.GetBoundingBox().ymin() - REFLECT_TOLERANCE,
			mesh.GetBoundingBox().zmin() - REFLECT_TOLERANCE};
	std::multimap<TCollision, bool> c;
	std::vector<std::pair<solid, bool> > currentsolids = { std::make_pair(defaultsolid, false) };
	if (GetCollisions(t,p,t,p2,c)){	// check for collisions of a vertical segment from p to lower border of bounding box
		for (auto i: c){
			auto sld = find_if(currentsolids.begin(), currentsolids.end(), [&i](const std::pair<solid, bool> s){ return s.first.ID == i.first.ID; });
			if (sld != currentsolids.end()) // if there is a collision with a solid already in the list, remove it from list
				currentsolids.erase(sld);
			else
				currentsolids.push_back(std::make_pair(GetSolid(i.first.ID), i.second)); // else add solid to list
		}
	}
	return currentsolids;
}

solid TGeometry::GetSolid(const double t, const double p[3]) const{
	// find first (highest-priority) solid that's not being ignored
	auto currentsolids = GetSolids(t, p);
//	for (auto s: currentsolids)
//		std::cout << s.first.name << " " << s.second << std::endl;
	auto sld = std::max_element(currentsolids.begin(), currentsolids.end(), [](const std::pair<solid, bool> s1, const std::pair<solid, bool> s2){ return s1.second || (!s2.second && s1.first.ID < s2.first.ID); });
//	std::cout << sld->first.name << " " << sld->second << std::endl;
	return sld->first;
}

solid TGeometry::GetSolid(const unsigned ID) const{
	auto sld = std::find_if(solids.begin(), solids.end(), [&ID](const solid &s){ return s.ID == ID; });
	if (sld == solids.end())
		throw std::runtime_error((boost::format("Could not find solid with ID %s") % ID).str());
	return *sld;
}
