package kuplrg

object Implementation extends Template {

  import Expr.*
  import RecDef.*
  import Value.*
  import Type.*
  import TypeInfo.*

  def subst(bodyTy: Type, name: List[String], rename: List[String]): Type = bodyTy match
    case UnitT => UnitT
    case NumT => NumT
    case BoolT => BoolT
    case StrT => StrT
    case IdT(tn, ts) => ts match
      case Nil => if (name.contains(tn)) IdT(rename(name.indexOf(tn)), Nil) else IdT(tn, ts)
      case _ => IdT(tn, ts.map(subst(_, name, rename)))
    case ArrowT(tns, ps, b) => ArrowT(tns, ps.map(subst(_, name, rename)), subst(b, name, rename))

  def substT(bodyTy: Type, name: List[String], ty: List[Type]): Type = bodyTy match
    case UnitT => UnitT
    case NumT => NumT
    case BoolT => BoolT
    case StrT => StrT
    case IdT(tn, ts) => ts match
      case Nil => if (name.contains(tn)) ty(name.indexOf(tn)) else IdT(tn, ts)
      case _ => IdT(tn, ts.map(substT(_, name, ty)))
    case ArrowT(tns, ps, b) => ArrowT(tns, ps.map(substT(_, name, ty)), substT(b, name, ty))

  def isSame(lty: Type, rty: Type): Boolean = (lty, rty) match
    case (UnitT, UnitT) => true
    case (NumT, NumT) => true
    case (BoolT, BoolT) => true
    case (StrT, StrT) => true
    case (IdT(ltn, Nil), IdT(rtn, Nil)) => ltn == rtn
    case (IdT(ltn, lts), IdT(rtn, rts)) =>
      if(lts.length != rts.length) error()
      (lts zip rts).forall((lt, rt) => isSame(lt, rt))
    case (ArrowT(ltNs, lpTy, lrTy), ArrowT(rtNs, rpTy, rrTy)) =>
      if((ltNs.length != rtNs.length) || (lpTy.length != rpTy.length)) error()
      (lpTy zip rpTy).forall((lp, rp) => isSame(lp, substT(rp, rtNs, ltNs.map(IdT(_, Nil)))))
    case _ => false

  def mustSame(lty: Type, rty: Type): Unit =
    if (!isSame(lty, rty)) error()

  def mustValid(ty: Type, tenv: TypeEnv): Type = ty match
    case UnitT => UnitT
    case NumT => NumT
    case BoolT => BoolT
    case StrT => StrT
    case IdT(tn, ts) => tenv.tys.getOrElse(tn, error()) match
      case TIAdt(vars, vs) =>
        for(t <- ts) mustValid(t, tenv)
        IdT(tn, ts)
      case TIVar => IdT(tn, ts)
    case ArrowT(tns, paramTys, retTy) =>
      for (pty <- paramTys) mustValid(pty, tenv.addTypeVars(tns))
      mustValid(retTy, tenv.addTypeVars(tns))
      ArrowT(tns, paramTys, retTy)

  def update(rec: RecDef, tenv: TypeEnv): TypeEnv = rec match
    case LazyVal(x, t, e) => tenv.addVar(x -> t)
    case RecFun(x, tns, ps, rty, b) => tenv.addVar(x -> ArrowT(tns, ps.map(_.ty), rty))
    case TypeDef(x, tns, vs) =>
      if (tenv.tys.contains(x)) error()
      val newTEnv = tenv.addTypeName(x, tns, vs)
      val retTy = newTEnv.addVars(vs.map(v => v.name -> ArrowT(tns, v.params.map(_.ty), IdT(x, tns.map(IdT(_, Nil))))))
      retTy

  def rule(rec: RecDef, tenv: TypeEnv): Unit = rec match
    case LazyVal(x, t, e) =>
      mustValid(t, tenv)
      mustSame(t, typeCheck(e, tenv))
    case RecFun(x, tns, ps, rty, b) =>
      for (tn <- tns) if(tenv.tys.contains(tn)) error()
      val newTEnv = tenv.addTypeVars(tns)
      val tys = ps.map(_.ty)
      for (ty <- tys) mustValid(ty, newTEnv)
      mustValid(rty, newTEnv)
      mustSame(rty, typeCheck(b, newTEnv.addVars(ps.map(p => p.name -> p.ty))))
    case TypeDef(x, tns, vs) =>
      for (tn <- tns) if(tenv.tys.contains(tn)) error()
      val newTEnv = tenv.addTypeVars(tns)
      for (v <- vs; vp <-v.params) mustValid(vp.ty, newTEnv)

  def ruleI(rec: RecDef, lenv: Env, renv: () => Env): Env = rec match
    case LazyVal(x, t, e) => lenv + (x -> ExprV(e, renv))
    case RecFun(x, tns, ps, rty, b) => lenv + (x -> CloV(ps.map(_.name), b, renv))
    case TypeDef(x, tns, vs) =>
      val vname = vs.map(_.name)
      val cv = vname.map(n => ConstrV(n))
      lenv ++ (vname zip cv)

  def eq(l: Value, r: Value): Boolean = (l, r) match
    case (UnitV, UnitV) => true
    case (NumV(l), NumV(r)) => l == r
    case (BoolV(l), BoolV(r)) => l == r
    case (StrV(l), StrV(r)) => l == r
    case (VariantV(ln, lv), VariantV(rn, rv)) =>
      (lv.length == rv.length) && (ln == rn) && (lv zip rv).forall((l, r) => eq(l, r))
    case _ => false

  def typeCheck(expr: Expr, tenv: TypeEnv): Type = expr match{
    case EUnit => UnitT
    case ENum(_) => NumT
    case EBool(_) => BoolT
    case EStr(_) => StrT
    case EId(x) => tenv.vars.getOrElse(x, error())
    case EAdd(l, r) =>
      mustSame(typeCheck(l, tenv), NumT)
      mustSame(typeCheck(r, tenv), NumT)
      NumT
    case EMul(l, r) =>
      mustSame(typeCheck(l, tenv), NumT)
      mustSame(typeCheck(r, tenv), NumT)
      NumT
    case EDiv(l, r) =>
      mustSame(typeCheck(l, tenv), NumT)
      mustSame(typeCheck(r, tenv), NumT)
      NumT
    case EMod(l, r) =>
      mustSame(typeCheck(l, tenv), NumT)
      mustSame(typeCheck(r, tenv), NumT)
      NumT
    case EConcat(l, r) =>
      mustSame(typeCheck(l, tenv), StrT)
      mustSame(typeCheck(r, tenv), StrT)
      StrT
    case EEq(l, r) =>
      mustSame(typeCheck(l, tenv), typeCheck(r, tenv))
      BoolT
    case ELt(l, r) =>
      mustSame(typeCheck(l, tenv), NumT)
      mustSame(typeCheck(r, tenv), NumT)
      BoolT
    case ESeq(l, r) =>
      val lftTy = typeCheck(l, tenv)
      val rgtTy = typeCheck(r, tenv)
      rgtTy
    case EIf(c, t, e) =>
      mustSame(typeCheck(c, tenv), BoolT)
      val thenTy = typeCheck(t, tenv)
      val elseTy = typeCheck(e, tenv)
      mustSame(thenTy, elseTy)
      thenTy
    case EVal(x, t, i, b) => t match
      case None => typeCheck(b, tenv.addVar(x -> typeCheck(i, tenv)))
      case Some(ty) =>
        mustSame(ty, typeCheck(i, tenv))
        typeCheck(b, tenv.addVar(x -> ty))
    case EFun(ps, b) =>
      val ptys = ps.map(_.ty)
      for (pty <- ptys) mustValid(pty, tenv)
      val rty = typeCheck(b, tenv.addVars(ps.map(p => p.name -> p.ty)))
      ArrowT(Nil, ptys, rty)
    case EApp(f, ts, as) => typeCheck(f, tenv) match
      case ArrowT(tvars, paramTys, retTy) =>
        for (t <- ts) mustValid(t, tenv)
        val aTy = as.map(typeCheck(_, tenv))
        if((paramTys.length != aTy.length) || (tvars.length != ts.length) || !(aTy zip paramTys).forall((at, pt) => isSame(at, substT(pt, tvars, ts)))) error()
        substT(retTy, tvars, ts)
      case t => error()
    case ERecDefs(ds, b) =>
      val finalTEnv = ds.foldLeft(tenv){(currentTEnv, recDef) => update(recDef, currentTEnv)}
      for (d <- ds) rule(d, finalTEnv)
      mustValid(typeCheck(b, finalTEnv), tenv)
    case EMatch(e, cs) => typeCheck(e, tenv) match
      case IdT(tn, tys) => tenv.tys.getOrElse(tn, error()) match
        case TIAdt(tvar, variants) =>
          val xs = cs.map(_.name).toSet
          if(cs.length != xs.size || variants.keySet != xs || tys.length != tvar.length) error()
          cs.map { case MatchCase(x, ps, b) =>
            typeCheck(b, tenv.addVars(ps zip variants(x).map(_.ty).map(substT(_, tvar, tys))))
          }.reduce((lty, rty) => {mustSame(lty, rty); lty})
        case ty => error()
      case _ => error()
    case EExit(ty, expr) =>
      mustValid(ty, tenv)
      mustSame(typeCheck(expr, tenv), StrT)
      ty
  }

  def interp(expr: Expr, env: Env): Value = expr match {
    case EUnit => UnitV
    case ENum(n) => NumV(n)
    case EBool(b) => BoolV(b)
    case EId(x) => env.getOrElse(x, error()) match
      case ExprV(expr, fenv) => interp(expr, fenv())
      case v => v
    case EStr(s) => StrV(s)
    case EAdd(l, r) => (interp(l, env), interp(r, env)) match
      case (NumV(l), NumV(r)) => NumV(l + r)
      case (l, r) => error()
    case EMul(l, r) => (interp(l, env), interp(r, env)) match
      case (NumV(l), NumV(r)) => NumV(l * r)
      case (l, r) => error()
    case EDiv(l, r) => (interp(l, env), interp(r, env)) match
      case (NumV(_), NumV(0)) => error()
      case (NumV(l), NumV(r)) => NumV(l / r)
      case (l, r) => error()
    case EMod(l, r) => (interp(l, env), interp(r, env)) match
      case (NumV(_), NumV(0)) => error()
      case (NumV(l), NumV(r)) => NumV(l % r)
      case (l, r) => error()
    case EConcat(l, r) => (interp(l, env), interp(r, env)) match
      case (StrV(l), StrV(r)) => StrV(l ++ r)
      case (l, r) => error()
    case EEq(l, r) => BoolV(eq(interp(l, env), interp(r, env)))
    case ELt(l, r) => (interp(l, env), interp(r, env)) match
      case (NumV(l), NumV(r)) => BoolV(l < r)
      case (l, r) => error()
    case ESeq(l, r) =>
      val lftV = interp(l, env)
      val rgtV = interp(r, env)
      rgtV
    case EIf(c, t, e) => interp(c, env) match
      case BoolV(true) => interp(t, env)
      case BoolV(false) => interp(e, env)
      case v => error()
    case EVal(x, t, i, b) => t match
      case None => interp(b, env + (x -> interp(i, env)))
      case Some(ty) => interp(b, env + (x -> interp(i, env)))
    case EFun(ps, b) => CloV(ps.map(_.name), b, () => env)
    case EApp(f, ts, as) => interp(f, env) match
      case CloV(ps, b, fenv) =>
        val arg = as.map(a => interp(a, env))
        if (ps.length != arg.length) error()
        interp(b, fenv() ++ (ps zip arg))
      case ConstrV(n) => VariantV(n, as.map(a => interp(a, env)))
      case v => error()
    case ERecDefs(ds, b) =>
      lazy val finalEnv: Env = ds.foldLeft(env) {
        (currentEnv, recDef) => ruleI(recDef, currentEnv, () => finalEnv)
      }
      interp(b, finalEnv)
    case EMatch(e, cs) => interp(e, env) match
      case VariantV(n, vs) => cs.find(_.name == n) match
        case Some(MatchCase(_, ps, b)) =>
          if (ps.length != vs.length) error()
          interp(b, env ++ (ps zip vs))
        case None => error()
      case v => error()
    case EExit(ty, expr) => expr match
      case EStr(s) => if(ty == StrT) StrV(s) else error()
      case _ => error()
  }
}
