/*******************************************************************************
 * CGoGN: Combinatorial and Geometric modeling with Generic N-dimensional Maps  *
 * Copyright (C), IGG Group, ICube, University of Strasbourg, France            *
 *                                                                              *
 * This library is free software; you can redistribute it and/or modify it      *
 * under the terms of the GNU Lesser General Public License as published by the *
 * Free Software Foundation; either version 2.1 of the License, or (at your     *
 * option) any later version.                                                   *
 *                                                                              *
 * This library is distributed in the hope that it will be useful, but WITHOUT  *
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or        *
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License  *
 * for more details.                                                            *
 *                                                                              *
 * You should have received a copy of the GNU Lesser General Public License     *
 * along with this library; if not, write to the Free Software Foundation,      *
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.           *
 *                                                                              *
 * Web site: http://cgogn.unistra.fr/                                           *
 * Contact information: cgogn@unistra.fr                                        *
 *                                                                              *
 *******************************************************************************/

#include <cgogn/core/types/cmap/cph3.h>
#include <cgogn/core/types/cmap/phi.h>

#include <cgogn/core/functions/traversals/face.h>

namespace cgogn
{

/***************************************************
 *              LEVELS MANAGEMENT                  *
 ***************************************************/

uint32 CPH3::dart_level(Dart d) const
{
	return (*dart_level_)[d.index];
}

void CPH3::set_dart_level(Dart d, uint32 l)
{
	(*dart_level_)[d.index] = l;
}

void CPH3::change_dart_level(Dart d, uint32 l)
{
	nb_darts_per_level_[dart_level(d)]--;
	nb_darts_per_level_[l]++;
	if (l > dart_level(d) && l > maximum_level_)
		maximum_level_ = l;
	if (l < dart_level(d))
	{
		while (nb_darts_per_level_[maximum_level_] == 0u)
		{
			--maximum_level_;
			nb_darts_per_level_.pop_back();
		}
	}
	(*dart_level_)[d.index] = l;
}

/***************************************************
 *             EDGE ID MANAGEMENT                  *
 ***************************************************/

uint32 CPH3::edge_id(Dart d) const
{
	return (*edge_id_)[d.index];
}

void CPH3::set_edge_id(Dart d, uint32 i)
{
	(*edge_id_)[d.index] = i;
}

uint32 CPH3::refinement_edge_id(Dart d, Dart e) const
{
	uint32 d_id = edge_id(d);
	uint32 e_id = edge_id(e);

	uint32 id = d_id + e_id;

	if (id == 0u)
		return 1u;
	else if (id == 1u)
		return 2u;
	else if (id == 2u)
	{
		if (d_id == e_id)
			return 0u;
		else
			return 1u;
	}
	// else if (id == 3)
	return 0u;
}

/***************************************************
 *             FACE ID MANAGEMENT                  *
 ***************************************************/

uint32 CPH3::face_id(Dart d) const
{
	return (*face_id_)[d.index];
}

void CPH3::set_face_id(Dart d, uint32 i)
{
	(*face_id_)[d.index] = i;
}

uint32 CPH3::refinement_face_id(const std::vector<Dart>& cut_path) const
{
	std::unordered_set<uint32> set_fid;
	for (Dart d : cut_path)
		set_fid.insert(face_id(d));
	uint32 result = 0;
	while (set_fid.find(result) != set_fid.end())
		++result;
	return result;
}

/***************************************************
 *                  EDGE INFO                      *
 ***************************************************/

uint32 CPH3::edge_level(Dart d) const
{
	cgogn_message_assert(dart_level(d) <= current_level_, "Access to a dart introduced after current level");
	return std::max(dart_level(d), dart_level(phi1(*this, d)));
}

Dart CPH3::edge_youngest_dart(Dart d) const
{
	cgogn_message_assert(dart_level(d) <= current_level_, "Access to a dart introduced after current level");
	Dart d2 = phi2(*this, d);
	if (dart_level(d) > dart_level(d2))
		return d;
	return d2;
}

/***************************************************
 *                  FACE INFO                      *
 ***************************************************/

uint32 CPH3::face_level(Dart d)
{
	cgogn_message_assert(dart_level(d) <= current_level_, "Access to a dart introduced after current level");
	if (current_level_ == 0)
		return 0;

	Dart it = d;
	Dart old = it;
	uint32 l_old = dart_level(old);
	uint32 fLevel = edge_level(it);
	do
	{
		it = phi1(*this, it);
		uint32 dl = dart_level(it);

		// compute the oldest dart of the face in the same time
		if (dl < l_old)
		{
			old = it;
			l_old = dl;
		}
		uint32 l = edge_level(it);
		fLevel = l < fLevel ? l : fLevel;
	} while (it != d);

	uint32 lsave = current_level_;
	current_level_ = fLevel;

	uint32 nbSubd = 0;
	it = old;
	uint32 eId = edge_id(old);
	uint32 init_dart_level = dart_level(it);
	do
	{
		++nbSubd;
		it = phi1(*this, it);
	} while (edge_id(it) == eId && dart_level(it) != init_dart_level);

	while (nbSubd > 1)
	{
		nbSubd /= 2;
		--fLevel;
	}

	current_level_ = lsave;

	return fLevel;
}

Dart CPH3::face_origin(Dart d)
{
	cgogn_message_assert(dart_level(d) <= current_level_, "Access to a dart introduced after current level");
	Dart p = d;
	uint32 pLevel = dart_level(p);
	uint32 lsave = current_level_;
	do
	{
		current_level_ = pLevel;
		p = face_oldest_dart(p);
		pLevel = dart_level(p);
	} while (pLevel > 0);
	current_level_ = lsave;
	return p;
}

Dart CPH3::face_oldest_dart(Dart d)
{
	cgogn_message_assert(dart_level(d) <= current_level_, "Access to a dart introduced after current level");
	Dart it = d;
	Dart oldest = it;
	uint32 l_old = dart_level(oldest);
	do
	{
		uint32 l = dart_level(it);
		if (l == 0)
			return it;

		if (l < l_old)
		//		if(l < l_old || (l == l_old && it < oldest))
		{
			oldest = it;
			l_old = l;
		}
		it = phi1(*this, it);
	} while (it != d);

	return oldest;
}

Dart CPH3::face_youngest_dart(Dart d)
{
	cgogn_message_assert(dart_level(d) <= current_level_, "Access to a dart introduced after current level");
	Dart it = d;
	Dart youngest = it;
	uint32 l_young = dart_level(youngest);
	do
	{
		uint32 l = dart_level(it);
		if (l == current_level_)
			return it;

		if (l > l_young)
		//		if(l < l_young || (l == l_young && it < youngest))
		{
			youngest = it;
			l_young = l;
		}
		it = phi1(*this, it);
	} while (it != d);

	return youngest;
}

/***************************************************
 *                 VOLUME INFO                     *
 ***************************************************/

uint32 CPH3::volume_level(Dart d)
{
	cgogn_message_assert(dart_level(d) <= current_level_, "Access to a dart introduced after current level");

	// The level of a volume is the
	// minimum of the levels of its faces

	Dart oldest = d;
	uint32 lold = dart_level(oldest);
	uint32 vLevel = std::numeric_limits<uint32>::max();

	foreach_incident_face(*this, CPH3::CMAP::Volume(d), [&](CPH3::CMAP::Face f) -> bool {
		uint32 fLevel = face_level(f.dart);
		vLevel = fLevel < vLevel ? fLevel : vLevel;
		Dart old = face_oldest_dart(f.dart);
		if (dart_level(old) < lold)
		{
			oldest = old;
			lold = dart_level(old);
		}
		return true;
	});

	MRCmap3 m2(m, vLevel);

	uint32 nbSubd = 0;
	Dart it = oldest;
	uint32 eId = m2.cph().edge_id(oldest);
	do
	{
		++nbSubd;
		it = phi<121>(m2, it);
	} while (m2.cph().edge_id(it) == eId && lold != m2.cph().dart_level(it));

	while (nbSubd > 1)
	{
		nbSubd /= 2;
		--vLevel;
	}

	return vLevel;
}

Dart CPH3::volume_oldest_dart(Dart d)
{
	cgogn_message_assert(m.cph().dart_level(d) <= current_level_, "Access to a dart introduced after current level");

	Dart oldest = d;
	uint32 l_old = m.cph().dart_level(oldest);
	foreach_incident_face(m, MRCmap3::Volume(oldest), [&](MRCmap3::Face f) -> bool {
		Dart old = face_oldest_dart(m, f.dart);
		uint32 l = m.cph().dart_level(old);
		if (l < l_old)
		{
			oldest = old;
			l_old = l;
		}
		return true;
	});

	return oldest;
}

Dart CPH3::volume_youngest_dart(Dart d)
{
	cgogn_message_assert(m.cph().dart_level(d) <= current_level_, "Access to a dart introduced after current level");

	Dart youngest = d;
	uint32 l_young = m.cph().dart_level(youngest);
	foreach_incident_face(m, MRCmap3::Volume(youngest), [&](MRCmap3::Face f) -> bool {
		Dart young = face_youngest_dart(m, f.dart);
		uint32 l = m.cph().dart_level(young);
		if (l > l_young)
		{
			youngest = young;
			l_young = l;
		}
		return true;
	});

	return youngest;
}

} // namespace cgogn
