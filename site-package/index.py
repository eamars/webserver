#!/usr/bin/env python3

HTML = """
<!DOCTYPE html>
<!-- saved from url=(0034)http://eamars.github.io/webserver/ -->
<html><head><meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
    <meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="chrome=1">
    <title>webhttpd - Yet Another Web Server by eamars</title>

    <link rel="stylesheet" href="./index_files/styles.css">
    <link rel="stylesheet" href="./index_files/github-light.css">
    <meta name="viewport" content="width=device-width, initial-scale=1, user-scalable=no">
    <!--[if lt IE 9]>
    <script src="//html5shiv.googlecode.com/svn/trunk/html5.js"></script>
    <![endif]-->
  <style type="text/css"></style></head>
  <body>
    <div class="wrapper">
      <header>
        <h1>webhttpd - Yet Another Web Server</h1>
        <p></p>

        <p class="view"><a href="https://github.com/eamars/webserver">View the Project on GitHub <small>eamars/webserver</small></a></p>


        <ul>
          <li><a href="https://github.com/eamars/webserver/zipball/master">Download <strong>ZIP File</strong></a></li>
          <li><a href="https://github.com/eamars/webserver/tarball/master">Download <strong>TAR Ball</strong></a></li>
          <li><a href="https://github.com/eamars/webserver">View On <strong>GitHub</strong></a></li>
        </ul>
      </header>
      <section>
        <h1>
<a id="webhttpd---yet-another-web-server" class="anchor" href="http://eamars.github.io/webserver/#webhttpd---yet-another-web-server" aria-hidden="true"><span class="octicon octicon-link"></span></a>webhttpd - Yet Another Web Server</h1>

<p><a href="https://travis-ci.org/eamars/webserver"><img src="https://travis-ci.org/eamars/webserver.svg?branch=master" alt="Build Status"></a></p>

<p>A simple HTTP server that provides minimal support of HTTP protocol.</p>

<p>Features:</p>

<ul>
<li>HTTP_GET &amp; HTTP_POST implementation</li>
<li>Basic CGI execution</li>
<li>Virtual hosts</li>
</ul>

<h2>
<a id="installation" class="anchor" href="http://eamars.github.io/webserver/#installation" aria-hidden="true"><span class="octicon octicon-link"></span></a>Installation</h2>

<p>Download and build from source:</p>

<pre><code>git clone git@github.com:eamars/webserver.git
</code></pre>

<p>webhttpd does not need any external dependencies.</p>

<p>Build webhttpd:</p>

<pre><code>make default
</code></pre>

<p>Build test</p>

<pre><code>make test
</code></pre>

<h2>
<a id="usage" class="anchor" href="http://eamars.github.io/webserver/#usage" aria-hidden="true"><span class="octicon octicon-link"></span></a>Usage</h2>

<p>You need to prepare the site package before starting webhttpd server. The site package folder requires site-config to provide basic information for starting a web server.</p>

<p>An example site-config is shown below</p>

<div class="highlight highlight-source-ini"><pre><span class="pl-c"># My site</span>

<span class="pl-c"># Server configuration</span>
<span class="pl-c"># Name of current running instance</span>
<span class="pl-k">server_name</span> = My site

<span class="pl-c"># Port to bind</span>
<span class="pl-k">server_port</span> = 80

<span class="pl-c"># dir</span>
<span class="pl-k">default_dir</span> = site-package

<span class="pl-c"># Default pages</span>
<span class="pl-k">default_index_page</span> = /index.html
<span class="pl-k">default_404_page</span> = /404.html
<span class="pl-k">default_favicon_image</span> = /favicon.ico
<span class="pl-k">default_robots_text</span> = /robots.txt</pre></div>

<p>To start the webhttpd server:</p>

<pre><code>./src/webhttpd start site-package
</code></pre>

<p>To terminate the webhttp server:</p>

<pre><code>./src/webhttpd stop site-package
</code></pre>

<h2>
<a id="contributions" class="anchor" href="http://eamars.github.io/webserver/#contributions" aria-hidden="true"><span class="octicon octicon-link"></span></a>Contributions</h2>

<ul>
<li>joyent for <a href="https://github.com/joyent/http-parser">http-parser</a>
</li>
</ul>
      </section>
      <footer>
        <p>This project is maintained by <a href="https://github.com/eamars">eamars</a></p>
        <p><small>Hosted on GitHub Pages — Theme by <a href="https://github.com/orderedlist">orderedlist</a></small></p>
      </footer>
    </div>
    <script src="./index_files/scale.fix.js"></script>



</body></html>
"""

print(HTML)
