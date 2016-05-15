/* There are several kinds of shape functions that have
   been used successfully for mesh adaptation.
   Each kind has a triangle and a tet variant.

   Symbols used below:
     Q_{tet} tetrahedral quality measure, [0,1]
     Q_{tri} triangular  quality measure, [0,1]
     V       tetrahedron volume
     A       triangle area
     A_i     area of triangle (i) of a tet
     l_i     length of edge (i) of a triangle or tet

   The first is the mean ratio measure, used by
   SCOREC and INRIA.
   For tets, the SCOREC implementation is the mean
   ratio cubed:

   Q_{tet} = 15552 V^2 / (\sum_{i=1}^6 l_i^2)^3

source:
   Li, Xiangrong, Mark S. Shephard, and Mark W. Beall.
   "3D anisotropic mesh adaptation by mesh modification."
   Computer methods in applied mechanics and engineering
   194.48 (2005): 4915-4950.

   The INRIA implementation for tets should be the
   actual mean ratio; the cube root of the SCOREC one:

   Q_{tet} = (36/(3^{1/3})) (V^{2/3}/(\sum_{i=1}^6 l_i^2))

   15552 = (36^3)/3

source:
   Loseille, Adrien, Victorien Menier, and Frederic Alauzet.
   "Parallel Generation of Large-size Adapted Meshes."
   Procedia Engineering 124 (2015): 57-69.
   (the normalization factor seems to have been misprinted
    here, it was not inverted like the rest of the formula)

   For triangles, the SCOREC variant is:

   Q_{tri} = 48 A^2 / (\sum_{i=1}^6 l_i^2)^2

   Another pair of measures that has been tried recently
   are some scale-invariant smooth measures associated with
   element stiffness matrix conditioning:

   Q_{tet} = V / (A_{rms})^{3/2}
   A_{rms} = \sqrt{(1/4)\sum_{i=1}^4 A_i^2}

   Q_{tri} = A / (l_{rms})^2
   l_{rms} = \sqrt{(1/3)\sum_{i=1}^3 l_i^2}
source:
   Shewchuk, J.
   "What is a good linear finite element?
    interpolation, conditioning, anisotropy,
    and quality measures"
   Proceedings of the 11th International Meshing Roundtable

   Some more treatment of quality measures can
   be found in
source:
   Liu, Anwei, and Barry Joe.
   "Relationship between tetrahedron shape measures."
   BIT Numerical Mathematics 34.2 (1994): 268-287.

   We will start off using the mean ratio cubed measures:
 */

INLINE Real triangle_mean_ratio_squared(Real a, Few<Real, 3> lsq) {
  Real s = 0.0;
  for (UInt i = 0; i < 3; ++i)
    s += lsq[i];
  return 48 * square(a) / square(s);
}

template <UInt dim>
INLINE Real triangle_mean_ratio_squared(
    Few<Vector<dim>, 3> p,
    Few<Vector<dim>, 2> b) {
  Real a = triangle_area(b);
  Few<Real, 3> lsq = triangle_edge_lengths_squared(p, b);
  return triangle_mean_ratio_squared(a, lsq);
}

INLINE Real tet_mean_ratio_cubed(Real v, Few<Real, 6> lsq) {
  Real s = 0.0;
  for (UInt i = 0; i < 6; ++i)
    s += lsq[i];
  return 15552.0 * square(v) / cube(s);
}

INLINE Real tet_mean_ratio_cubed(
    Few<Vector<3>, 4> p,
    Few<Vector<3>, 3> b) {
  Real v = tet_volume(b);
  Few<Real, 6> lsq = tet_edge_lengths_squared(p, b);
  return tet_mean_ratio_cubed(v, lsq);
}