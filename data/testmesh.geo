SetFactory("OpenCASCADE");
lc = 2.5;
Point(1) = {0, 0, 0, lc};
Point(2) = {0, 5, 0, lc};
Point(3) = {5, 5, 0, lc};
Point(4) = {5, 0, 0, lc};
Point(5) = {0, 0, 5, lc};
Point(6) = {5, 0, 5, lc};
Point(7) = {5, 5, 5, lc};
Point(8) = {0, 5, 5, lc};

Line (1) = {1, 2};
Line (2) = {2, 3};
Line (3) = {3, 4};
Line (4) = {4, 1};
Line (5) = {5, 6};
Line (6) = {6, 7};
Line (7) = {7, 8};
Line (8) = {8, 5};
Line (9) = {4, 6};
Line (10) = {1, 5};
Line (11) = {3, 7};
Line (12) = {2, 8};

Circle(20) = {2.5, 2.5, 5, 1, 0, 2*Pi};

Curve Loop (1) = {1, 2, 3, 4};
Curve Loop (2) = {5, 6, 7, 8};
Curve Loop (3) = {20};
Curve Loop (4) = {9, -5, -10, -4};
Curve Loop (5) = {11, -6, -9, -3};
Curve Loop (6) = {12, -7, -11, -2};
Curve Loop (7) = {10, -8, -12, -1};

Plane Surface (1) = {1};
Plane Surface (2) = {2, -3};
Plane Surface (3) = {3};
Plane Surface (4) = {4};
Plane Surface (5) = {5};
Plane Surface (6) = {6};
Plane Surface (7) = {7};

Physical Surface (1) = {1, 2, 4, 5, 6, 7};
Physical Surface (2) = {3};
