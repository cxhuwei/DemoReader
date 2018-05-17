#if !defined(AFX_IMAGESCALINGMATH_H__5182C50F_1319_47D5_A459_19D2BFAB4587__INCLUDED_)
#define AFX_IMAGESCALINGMATH_H__5182C50F_1319_47D5_A459_19D2BFAB4587__INCLUDED_
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif
class   SS_Point2
{
public:
  SS_Point2();
  SS_Point2(float const x_, float const y_);
  SS_Point2(SS_Point2 const &p);
  float Distance(SS_Point2 const p2);
  float Distance(float const x_, float const y_);
  float x,y;
};

class   SS_Rect2
{
public:
  SS_Rect2();
  SS_Rect2(float const x1_, float const y1_, float const x2_, float const y2_);
  SS_Rect2(SS_Point2 const &bl, SS_Point2 const &tr);
  SS_Rect2(SS_Rect2 const &p);

  float Surface() const;
  SS_Rect2 CrossSection(SS_Rect2 const &r2) const;
  SS_Point2 Center() const;
  float Width() const;
  float Height() const;

  SS_Point2 botLeft;
  SS_Point2 topRight;
};
#endif // !defined(AFX_IMAGESCALINGMATH_H__5182C50F_1319_47D5_A459_19D2BFAB4587__INCLUDED_)
