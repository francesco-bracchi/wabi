// v0 no fexpr

wabi_val
wabi_eval(wabi_store store,
          wabi_val expr,
          wabi_env env)
{
  switch(wabi_val_type(expr)) {
  case WABI_TAG_PAIR:
    wabi_val car = wabi_car(expr);
    wabi_val cdr = wabi_cdr(expr);
    // todo make a loop?
    wabi_val combiner = wabi_eval(store, car, env);
    return wabi_apply(store, combiner, cdr, env);
  case WABI_TAG_SYMBOL:
    return wabi_env_lookup(env, expr);
  default:
    return expr;
  }
}

wabi_val
wabi_eval_all(wabi_store store,
              wabi_val data,
              wabi_env env)
{
  switch(wabi_val_type(arguments)) {
  case WABI_TYPE_PAIR:
    wabi_val car = wabi_eval(store, wabi_car(data), env);
    wabi_val cdr = wabi_eval_all(store, wabi_cdr(data), env);
    return wabi_cons(store, car, cdr);
  }
  case WABI_TYPE_NIL:
    return data;
}


wabi_val
wabi_apply(wabi_store store,
           wabi_val combiner,
           wabi_val arguments,
           wabi_env env)
{

  arguments = wabi_eval_all(store, arguments, env);
  return wabi_call_combiner(store, combiner, arguments, env);
}

wabi_val
wabi_call_combiner(wabi_store store, wabi_val combiner, wabi_val arguments, wabi_env env)
{
  if(WABI_COMBINER_IS_BUILTIN(combiner)) {
    wabi_builtin_fun_type fun = (wabi_builtin_fun_type) (*combiner & WABI_VALUE_MASK);
    return fun(store, arguments, env);
  }
  else {
    wabi_combiner_derived comb = (wabi_combiner_derived) combiner;
    wabi_env new_env = wabi_env_extend(store, comb->static_env);
    wabi_bind(new_env, arguments, comb->arguments);
    wabi_env_def(new_env, comb->caller_env_name, env);
    return wabi_eval(store, comb->body, new_env);
  }
}

// V1 fexprs
wabi_val
wabi_apply(wabi_store store,
           wabi_val combiner,
           wabi_val arguments,
           wabi_env env)
{

  if(WABI_COMBINER_IS_APPLICATIVE(combiner)) {
    arguments = wabi_eval_all(store, arguments, env);
  }
  return wabi_call_combiner(store, combiner, arguments, env);
}
