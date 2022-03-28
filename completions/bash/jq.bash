# jq(1) completion

_jq()
{
  local cur prev opts
  _get_comp_words_by_ref cur prev

  opts=(
    '--version'
    '--seq'
    '--stream'
    '--slurp'
    '-s'
    '--raw-input'
    '-R'
    '--null-input'
    '-n'
    '--compact-output'
    '-c'
    '--tab'
    '--indent'
    '--color-output'
    '-C'
    '--monochrome-output'
    '-M'
    '--ascii-output'
    '-a'
    '--unbuffered'
    '--sort-keys'
    '-S'
    '--raw-output'
    '-r'
    '--join-output'
    '-j'
    '-f'
    '--from-file'
    '-L'
    '-e'
    '--exit-status'
    '--arg'
    '--argjson'
    '--slurpfile'
    '--argfile'
    '--args'
    '--jsonargs'
    '--run-tests'
  )

  case $prev in
    -f|--from-file|--run-tests)
      COMPREPLY=($(compgen -f -- "$cur"))
      return
      ;;
    -L)
      COMPREPLY=($(compgen -d -- "$cur"))
      return
      ;;
    --arg|--argjson|--slurpfile|--argfile|--args|--jsonargs)
      return
      ;;
  esac

  if [[ "${COMP_WORDS[COMP_CWORD-2]}" == --arg?(json) ]]; then
    return
  fi

  if [[ "${COMP_WORDS[COMP_CWORD-2]}" == --@(slurp|arg)file ]]; then
    COMPREPLY=($(compgen -f -- "$cur"))
    return
  fi

  COMPREPLY=($(compgen -W "${opts[*]}" -- "$cur"))

} &&
complete -F _jq jq
