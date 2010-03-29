require "ivrworx"
require "play_phrase"

this = {};

function test_play(this,i)
print(i)
end

function test_answer()
end

function test_hangup()
end

this["play"]   = test_play
this["answer"] = test_answer
this["hangup"] = test_hangup


play_phrase.play_number(1)