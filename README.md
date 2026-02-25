# how - Neo-UNIX command for asking questions!

![ManpageExample](https://github.com/Wrench56/how/blob/main/assets/manpage-example.png?raw=true)

Well, I guess I am writing LLM wrappers now...

---

## Shell as an AI...

Well, obviously this started as a joke. This whole thing started off as "Hey, what is the best scripting language out there?"

Well, somehow this has evolved into "AI as a shell" and finally "Shell as an AI". So I figured, why not lol. Five-ish hours later, I have a working `how` command. I coined the term Neo-UNIX to refer to UNIX tools that use modernized infrastructure. As such, this is the (first?!) Neo-UNIX tool. It's usage is fairly simple: you type in `how` followed by your question.

To set it up, you have to have a Claude API key. Set the environmental variable `CLAUDE_API_KEY` using:

```sh
export CLAUDE_API_KEY="sk-ant-..."
```

Example:

`how to create subprocesses on Linux in C`

By default, it opens a manpage that describes in a "manpage-manner" your question. See the advanced users section to figure out how to modify this behaviour.

## Installation

Simply run

```sh
sudo make install
```

to install the executable. You can also install locally using:

```sh
make PREFIX=$HOME/.local install
```

## Advanced users

You can modify the system prompt and response viewer. If one prefers plaintext over manpages, they can override the `HOW_VIEWER_CMD` environmental varaible and set it to their preferred pager. The output of the request is **piped** into the pager/viewer. As such, something like `less -R` works perfectly. However, currently, setting the viewer without setting the system prompt is still going to spit out ROFF. I advise you as such to change `HOW_SYSTEM_PROMPT` and ask for your preferred format.

### Examples

#### Less as Viewer

```sh
export HOW_VIEWER_CMD="less -R"
```

#### Multiline

```sh
how <<EOF
This allows you to input multiline text
You can type until you type E O F without the spaces.
Like below:
EOF
```

`how` reads from stdin when it is not attached to a TTY.

#### Wizardry with stdin

```sh
cat myfile.c | how does it do it?
```

When stdin is present, `how` will first submit your arguments then whatever is present in stdin.

## Security

The API keys are only stored on your machine. I never ever send it to anyone except the official LLM's website. I have a solid HTTPS implementation with multiple verification steps. You can take a look at `https.c` and see for yourself.
