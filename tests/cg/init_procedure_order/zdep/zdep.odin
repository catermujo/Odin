package zdep

ready: bool

@(init)
zdep_init :: proc "contextless" () {
	ready = true
}
